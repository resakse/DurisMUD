#!/bin/bash
################################################################################
# DurisMUD Player File Batch Converter
#
# Converts all player files from big-endian to little-endian format (After Xanadin's commit).
# Creates backups before conversion and provides rollback capability.
#
# Usage:
#   ./convert_all_pfiles.sh [OPTIONS]
#
# Options:
#   --dry-run       Test mode, no files modified
#   --verbose       Show detailed output
#   --rollback      Restore all files from .preconvert backups
#   --help          Show this help message
#
# Safety:
#   - Creates .preconvert backup for each file before conversion
#   - Logs all operations to conversion.log
#   - Can be rolled back with --rollback
#   - Skips files that appear already converted
#
################################################################################

set -e  # Exit on error

# Configuration
CONVERTER="./pfile_converter"
PLAYERS_DIR="Players"
LOG_FILE="logs/conversion.log"
BACKUP_SUFFIX=".preconvert"

# Options
DRY_RUN=0
VERBOSE=0
ROLLBACK=0

# Statistics
TOTAL_FILES=0
CONVERTED_FILES=0
SKIPPED_FILES=0
FAILED_FILES=0
ROLLED_BACK=0

################################################################################
# Functions
################################################################################

log() {
    local msg="[$(date '+%Y-%m-%d %H:%M:%S')] $*"
    echo "$msg" | tee -a "$LOG_FILE"
}

log_quiet() {
    echo "[$(date '+%Y-%m-%d %H:%M:%S')] $*" >> "$LOG_FILE"
}

error() {
    log "ERROR: $*"
}

verbose() {
    if [ $VERBOSE -eq 1 ]; then
        log "$*"
    else
        log_quiet "$*"
    fi
}

print_usage() {
    cat <<EOF
DurisMUD Player File Batch Converter

Usage: $0 [OPTIONS]

Options:
  --dry-run       Test mode, validate files without converting
  --verbose       Show detailed output for each file
  --rollback      Restore all files from .preconvert backups
  --help          Show this help message

Examples:
  $0 --dry-run --verbose    # Test conversion on all files
  $0                         # Convert all files (creates backups)
  $0 --rollback             # Undo conversion, restore backups

Safety:
  - Creates .preconvert backup for each file before conversion
  - All operations logged to $LOG_FILE
  - Can be rolled back with --rollback option
  - Skips files that appear already converted (little-endian)

EOF
}

check_prerequisites() {
    log "Checking prerequisites..."

    if [ ! -x "$CONVERTER" ]; then
        error "Converter not found or not executable: $CONVERTER"
        error "Please compile it first: gcc -o pfile_converter arih/pfile_converter.c -Wall -O2"
        exit 1
    fi

    if [ ! -d "$PLAYERS_DIR" ]; then
        error "Players directory not found: $PLAYERS_DIR"
        exit 1
    fi

    log "Prerequisites OK"
}

is_big_endian_file() {
    local file="$1"

    # Check if file exists and is at least 41 bytes
    if [ ! -f "$file" ] || [ $(stat -f%z "$file" 2>/dev/null || stat -c%s "$file") -lt 41 ]; then
        return 1
    fi

    # Try to validate with converter in dry-run mode
    if "$CONVERTER" --dry-run "$file" >/dev/null 2>&1; then
        # If validation succeeds, it's a valid old format file
        return 0
    else
        # If validation fails, might already be converted or corrupted
        return 1
    fi
}

convert_file() {
    local file="$1"
    local backup="${file}${BACKUP_SUFFIX}"

    verbose "Processing: $file"

    # Skip if backup already exists
    if [ -f "$backup" ]; then
        verbose "  Skipped (already has .preconvert backup - use --rollback to restore)"
        SKIPPED_FILES=$((SKIPPED_FILES + 1))
        return 0
    fi

    # Validate file format
    if ! is_big_endian_file "$file"; then
        verbose "  Skipped (not a valid big-endian pfile or already converted)"
        SKIPPED_FILES=$((SKIPPED_FILES + 1))
        return 0
    fi

    if [ $DRY_RUN -eq 1 ]; then
        log "  [DRY-RUN] Would convert: $file"
        CONVERTED_FILES=$((CONVERTED_FILES + 1))
        return 0
    fi

    # Create backup
    verbose "  Creating backup: $backup"
    if ! cp "$file" "$backup"; then
        error "  Failed to create backup: $backup"
        FAILED_FILES=$((FAILED_FILES + 1))
        return 1
    fi

    # Convert file (in-place)
    verbose "  Converting..."
    if "$CONVERTER" "$file" >> "$LOG_FILE" 2>&1; then
        log "  ✓ Converted: $file"
        CONVERTED_FILES=$((CONVERTED_FILES + 1))

        # Log file size change
        local old_size=$(stat -f%z "$backup" 2>/dev/null || stat -c%s "$backup")
        local new_size=$(stat -f%z "$file" 2>/dev/null || stat -c%s "$file")
        local diff=$((new_size - old_size))
        verbose "  Size: $old_size -> $new_size bytes (${diff:+'+'}$diff)"
    else
        error "  Failed to convert: $file"
        # Restore from backup
        verbose "  Restoring from backup..."
        cp "$backup" "$file"
        rm "$backup"
        FAILED_FILES=$((FAILED_FILES + 1))
        return 1
    fi
}

rollback_file() {
    local file="$1"
    local backup="${file}${BACKUP_SUFFIX}"

    if [ ! -f "$backup" ]; then
        verbose "  No backup found: $backup"
        return 0
    fi

    if [ $DRY_RUN -eq 1 ]; then
        log "  [DRY-RUN] Would restore: $file from $backup"
        ROLLED_BACK=$((ROLLED_BACK + 1))
        return 0
    fi

    verbose "  Restoring: $file from $backup"
    if cp "$backup" "$file" && rm "$backup"; then
        log "  ✓ Restored: $file"
        ROLLED_BACK=$((ROLLED_BACK + 1))
    else
        error "  Failed to restore: $file"
        FAILED_FILES=$((FAILED_FILES + 1))
        return 1
    fi
}

find_player_files() {
    # Find all player files in alphabetical subdirectories
    # Exclude: .bak, .old, .converted, .backup files and Backup/Kingdoms/Assocs dirs
    find "$PLAYERS_DIR" -type f \
        ! -name "*.bak" \
        ! -name "*.old" \
        ! -name "*.converted" \
        ! -name "*.backup" \
        ! -name "*backup*" \
        ! -name ".*" \
        ! -path "*/Backup/*" \
        ! -path "*/Kingdoms/*" \
        ! -path "*/Assocs/*" \
        -path "$PLAYERS_DIR/[a-z]/*" \
        2>/dev/null | sort
}

################################################################################
# Main
################################################################################

# Parse arguments
while [ $# -gt 0 ]; do
    case "$1" in
        --dry-run)
            DRY_RUN=1
            ;;
        --verbose)
            VERBOSE=1
            ;;
        --rollback)
            ROLLBACK=1
            ;;
        --help)
            print_usage
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            print_usage
            exit 1
            ;;
    esac
    shift
done

# Start logging
echo "" >> "$LOG_FILE"
log "=========================================="
log "DurisMUD Player File Batch Converter"
if [ $DRY_RUN -eq 1 ]; then
    log "MODE: DRY RUN (no files will be modified)"
elif [ $ROLLBACK -eq 1 ]; then
    log "MODE: ROLLBACK (restoring from backups)"
else
    log "MODE: CONVERSION (creating backups and converting)"
fi
log "=========================================="

# Check prerequisites
check_prerequisites

# Find all player files
log "Scanning for player files..."
PLAYER_FILES=$(find_player_files)
TOTAL_FILES=$(echo "$PLAYER_FILES" | wc -l | tr -d ' ')

if [ -z "$PLAYER_FILES" ] || [ "$TOTAL_FILES" -eq 0 ]; then
    log "No player files found in $PLAYERS_DIR/[a-z]/"
    exit 0
fi

log "Found $TOTAL_FILES player file(s)"
echo ""

# Process files
if [ $ROLLBACK -eq 1 ]; then
    log "Rolling back conversions..."
    while IFS= read -r file; do
        rollback_file "$file"
    done <<< "$PLAYER_FILES"
else
    log "Converting player files..."
    while IFS= read -r file; do
        convert_file "$file"
    done <<< "$PLAYER_FILES"
fi

# Print summary
echo ""
log "=========================================="
log "SUMMARY"
log "=========================================="
log "Total files:     $TOTAL_FILES"
if [ $ROLLBACK -eq 1 ]; then
    log "Rolled back:     $ROLLED_BACK"
else
    log "Converted:       $CONVERTED_FILES"
    log "Skipped:         $SKIPPED_FILES"
fi
log "Failed:          $FAILED_FILES"
log "=========================================="

if [ $DRY_RUN -eq 1 ]; then
    log ""
    log "DRY RUN complete. No files were modified."
    log "Run without --dry-run to perform actual conversion."
elif [ $ROLLBACK -eq 1 ]; then
    log ""
    log "Rollback complete."
else
    log ""
    log "Conversion complete!"
    log "Backups saved with $BACKUP_SUFFIX suffix"
    log "Use '$0 --rollback' to restore original files if needed"
fi

log "Log file: $LOG_FILE"

# Exit with error if any failures
if [ $FAILED_FILES -gt 0 ]; then
    exit 1
fi

exit 0
