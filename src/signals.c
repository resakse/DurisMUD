/*
   ***************************************************************************
   *  File: signals.c                                          Part of Duris *
   *  Usage: Signal Trapping.                                                  *
   *  Copyright  1990, 1991 - see 'license.doc' for complete information.      *
   *  Copyright 1994 - 2008 - Duris Systems Ltd.                             *
   *************************************************************************** 
 */

#include <stdio.h>
#include <sys/time.h>
#include <sys/wait.h>

#include "prototypes.h"
#include "structs.h"
#include "utils.h"

extern void exit(int);

/*
   external variables 
 */

extern int tics;
extern bool game_booted;
extern int shutdownflag;

//extern pid_t lookup_host_process;
extern pid_t lookup_ident_process;
void     reap(int sig);

void     shutdown_request(int);
void     shutdown_notice(int);
void     hupsig(int);
void     logsig(int);
void     reap(int);
void     checkpointing(int);

void signal_setup(void)
{
  struct itimerval itime;
  struct timeval interval;

  signal(SIGUSR2, shutdown_request);
  signal(SIGUSR1, shutdown_notice);

  /*
     just to be on the safe side: 
   */

  signal(SIGHUP, hupsig);
  signal(SIGPIPE, SIG_IGN);
  signal(SIGINT, hupsig);
  signal(SIGALRM, logsig);
  signal(SIGTERM, hupsig);
  /* new by fafhrd 11/28/99 */
  signal(SIGCHLD, reap);

  /*
     set up the deadlock-protection
   */

  // Start timer 900 sec after boot starts (15 min).
  interval.tv_sec = 900;
  interval.tv_usec = 0;
  itime.it_value = interval;
  // And have timer check every 15 minutes.
  itime.it_interval = interval;
  // Changing this to 5 min since we don't need to hang for 15 min to know we're stuck.
  itime.it_interval.tv_sec = 300;
  setitimer(ITIMER_VIRTUAL, &itime, 0);
  signal(SIGVTALRM, checkpointing);
}

// This handles a nothing-happens over a period of time.
void checkpointing(int signum)
{

  if( !tics )
  {
    logit(LOG_EXIT, "CHECKPOINT shutdown: tics not updated");
    // The reason for this, is that we don't want to reboot into a hung-during-boot situation.
    // In other words, if the mud hangs during a boot, we just want to die completely until it's fixed.
    if( game_booted )
    {
      exit( 56 );
    }
    else
    {
      exit( -1 );
    }
  }
  else
  {
    tics = 0;
  }

  // Add this linefor signal 26 -DCL
  signal(SIGVTALRM, checkpointing);
}

void shutdown_notice(int signum)
{

  logit(LOG_STATUS, "Received USR1 - shutdown notice");
  shutdownflag = 2;
  signal(SIGUSR1, shutdown_notice);
}

void shutdown_request(int signum)
{

  logit(LOG_STATUS, "Received USR2 - shutdown request");
  shutdownflag = 1;
}

/*
   kick out players etc 
 */
void hupsig(int signum)
{

  logit(LOG_EXIT, "Received SIGHUP, SIGINT, or SIGTERM. Shutting down");
  raise(SIGSEGV);
#if 0
  exit(-1);
#endif
}

void logsig(int signum)
{
  logit(LOG_SYS, "Signal received. Ignoring.");
}

/* This should do the trick... fafhrd 11/28/99 */

/* clean up our zombie kids to avoid defunct processes */
void reap(int sig)
{
  while (waitpid(-1, NULL, WNOHANG) > 0) ;

  signal(SIGCHLD, reap);

}

void reaper(int signum)
{
}
