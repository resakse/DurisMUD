#!/bin/bash

# Parse command line arguments
DEV_MODE=0
if [[ "$1" == "--dev" ]]; then
  DEV_MODE=1
  echo "Running in DEV mode - using TEST_MUD database"
fi

RESULT=53
STOP_REASON="initial bootup"

ulimit -c unlimited

# Extract database credentials from sql.h before loop starts
if [ -f "src/sql.h" ]; then
  if [ $DEV_MODE -eq 1 ]; then
    # Get TEST_MUD credentials (first match in ifdef)
    DB_HOST=$(grep '#define DB_HOST' src/sql.h | sed -n '1p' | sed 's/.*"\(.*\)".*/\1/')
    DB_USER=$(grep '#define DB_USER' src/sql.h | sed -n '1p' | sed 's/.*"\(.*\)".*/\1/')
    DB_PASSWD=$(grep '#define DB_PASSWD' src/sql.h | sed -n '1p' | sed 's/.*"\(.*\)".*/\1/')
    DB_NAME=$(grep '#define DB_NAME' src/sql.h | sed -n '1p' | sed 's/.*"\(.*\)".*/\1/')
  else
    # Get production credentials (second match in else clause)
    DB_HOST=$(grep '#define DB_HOST' src/sql.h | sed -n '2p' | sed 's/.*"\(.*\)".*/\1/')
    DB_USER=$(grep '#define DB_USER' src/sql.h | sed -n '2p' | sed 's/.*"\(.*\)".*/\1/')
    DB_PASSWD=$(grep '#define DB_PASSWD' src/sql.h | sed -n '2p' | sed 's/.*"\(.*\)".*/\1/')
    DB_NAME=$(grep '#define DB_NAME' src/sql.h | sed -n '2p' | sed 's/.*"\(.*\)".*/\1/')
  fi
  echo "Database: $DB_NAME on $DB_HOST"

  # Create server_reboots table if it doesn't exist
  mysql -h "$DB_HOST" -u "$DB_USER" -p"$DB_PASSWD" "$DB_NAME" -e "
    CREATE TABLE IF NOT EXISTS server_reboots (
      id INT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,
      boot_time INT NOT NULL,
      shutdown_time INT NOT NULL,
      uptime_seconds INT NOT NULL,
      shutdown_type VARCHAR(50) NOT NULL DEFAULT 'unknown',
      initiated_by VARCHAR(255) NULL,
      reason TEXT NULL,
      created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
      INDEX idx_boot_time (boot_time),
      INDEX idx_shutdown_time (shutdown_time),
      INDEX idx_created_at (created_at),
      INDEX idx_shutdown_type (shutdown_type)
    ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
  " 2>/dev/null
else
  echo "Warning: src/sql.h not found, skipping database operations"
  DB_HOST=""
fi

while [[ $RESULT != 0 && $RESULT != 55 ]]; do
	DATESTR=`date +%C%y.%m.%d-%H.%M.%S`

  if [[ $RESULT == 53 || $RESULT == 57 ]]; then
    if [ -f src/dms_new ]; then
			if [ -f dms ]; then
		  	mv dms dms.$DATESTR
			fi
      mv src/dms_new dms
    fi
  fi

  if [ -d logs/log ]; then
    #LOGNAME=`date +%b%d-%H%M`
    mkdir logs/old-logs/$DATESTR
    mv logs/log/* logs/old-logs/$DATESTR
    if [ -f core ]; then
      mv core core.$DATESTR
    fi
  fi

  echo "Backing up pfiles..."
  ./backup_pfiles.sh

  echo "Generating list of function names.."
  nm --demangle dms | grep " T " | sed -e 's/[(].*//g' > lib/misc/event_names

	if [ -f /usr/bin/sendemail ]; then
		if [ -f /logs/old-logs/$DATESTR/exit ]; then
			/usr/bin/sendEmail -t alert@durismud.com \
				-f mud@durismud.com -u "Duris Booting..." \
				-m "Mud booting at ${DATESTR}, previous shutdown reason: ${STOP_REASON} [${RESULT}]." \
      	-a "logs/old-logs/${DATESTR}/exit"
		else
			/usr/bin/sendEmail -t alert@durismud.com \
				-f mud@durismud.com -u "Duris Booting..." \
				-m "Mud booting at ${DATESTR}, previous shutdown reason: ${STOP_REASON} [${RESULT}]."
		fi
	fi

  # Record boot time (will be used for shutdown record later)
  BOOT_TIME=$(date +%s)

  echo "Starting duris..."
  ./dms 7777 # > dms.out

	# capture the exit code
  RESULT=${PIPESTATUS[0]}

	# determine the reason for shutting down
	case $RESULT in
		0) STOP_REASON="shutdown";;
		139) STOP_REASON="crash";;
		52) STOP_REASON="reboot";;
		53) STOP_REASON="copyover reboot";;
		54) STOP_REASON="auto reboot";;
		55) STOP_REASON="pwipe shutdown";;
		56) STOP_REASON="mud hung reboot";;
		57) STOP_REASON="auto reboot with copyover";;
		*) STOP_REASON="unknown";;
	esac

	echo "Mud stopped, reason: ${STOP_REASON} [${RESULT}]"

  # Log shutdown to database for server reboot tracking
  if [ -n "$DB_HOST" ]; then
    SHUTDOWN_TIME=$(date +%s)

    # Default values for database
    DB_SHUTDOWN_TYPE="unknown"
    INITIATED_BY=""
    SHUTDOWN_REASON=""

    # Parse shutdown info file if it exists
    if [ -f "logs/shutdown_info.txt" ]; then
      SHUTDOWN_INFO=$(cat "logs/shutdown_info.txt")
      INITIATED_BY=$(echo "$SHUTDOWN_INFO" | cut -d'|' -f1)
      SHUTDOWN_REASON=$(echo "$SHUTDOWN_INFO" | cut -d'|' -f2)
      # Delete the file after reading
      rm -f "logs/shutdown_info.txt"
    fi

    # Map exit code to database shutdown_type enum
    case $RESULT in
      0) DB_SHUTDOWN_TYPE="shutdown";;
      52) DB_SHUTDOWN_TYPE="reboot";;
      53) DB_SHUTDOWN_TYPE="copyover";;
      54) DB_SHUTDOWN_TYPE="autoreboot";;
      55) DB_SHUTDOWN_TYPE="pwipe";;
      56) DB_SHUTDOWN_TYPE="hung";;
      57) DB_SHUTDOWN_TYPE="autoreboot_copyover";;
      139) DB_SHUTDOWN_TYPE="crash";;
      *) DB_SHUTDOWN_TYPE="unknown";;
    esac

    # Calculate MUD uptime (shutdown_time - boot_time)
    MUD_UPTIME=$((SHUTDOWN_TIME - BOOT_TIME))

    # Insert a complete reboot record (boot + shutdown)
    mysql -h "$DB_HOST" -u "$DB_USER" -p"$DB_PASSWD" "$DB_NAME" -e "
      INSERT INTO server_reboots
        (boot_time, shutdown_time, uptime_seconds, shutdown_type, initiated_by, reason)
      VALUES
        (${BOOT_TIME}, ${SHUTDOWN_TIME}, ${MUD_UPTIME}, '${DB_SHUTDOWN_TYPE}',
         IF('${INITIATED_BY}' = '', NULL, '${INITIATED_BY}'),
         IF('${SHUTDOWN_REASON}' = '', NULL, '${SHUTDOWN_REASON}'));
    " 2>/dev/null
    echo "Logged reboot: ${MUD_UPTIME}s uptime, type: ${DB_SHUTDOWN_TYPE}"
  fi

  echo "Sleeping 10 seconds to prevent coreflood..."
  sleep 10
done

if [ $RESULT == 55 ]; then
  echo "Wiping player data..."
  ./Players/wipers/wipe_it_all
  echo "Moving player-logs to backup.."
  if [ -d logs/player-log ]; then
    #LOGNAME=`date +%b%d-%H%M`
    mkdir logs/player-log/$DATESTR
    mv logs/player-log/* logs/player-log/$DATESTR
  fi
  echo "Wiped!"
fi

if [ -f /usr/bin/sendemail ]; then
	/usr/bin/sendEmail -t alert@durismud.com \
		-f mud@durismud.com -u "Duris Shutdown..." \
		-m "Mud shutdown at ${DATESTR}, shutdown reason: ${STOP_REASON} [${RESULT}]."
fi

