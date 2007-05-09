/* $Id: s710d.c,v 1.3 2002/10/10 10:11:13 dave Exp $ */

#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <syslog.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include "s710.h"
#include "config.h"

/* globals */

static int           gSigFlag;

/* function declarations */

static void          signal_handler   ( int signum );

/* main program */

int
main ( int argc, char **argv )
{
  pid_t           pid;
  char            path[PATH_MAX];
  char           *filedir;
  files_t         file;
  S710_Driver     driver;
  int             ok;

  ok = driver_init ( argc, argv, &driver );

  if ( ok != 1 ) {
    printf("usage: %s [-d driver] [device file]\n",argv[0]);
    printf("\tdriver may be either serial, ir, or usb\n");
    printf("\tdevice file is required for serial and ir drivers.\n");
    exit(1);
  }

  if ( (filedir = getenv("S710_FILEDIR")) != NULL ) {
    filedir = realpath(filedir,path);
  } else {
    filedir = S710_FILEDIR;
  }

  /* fork */

  if ( (pid = fork()) == 0 ) {

    /* print syslog messages to stderr as well (debugging) */

    openlog("s710d",LOG_CONS|LOG_PID|LOG_PERROR,LOG_USER);
    syslog(LOG_NOTICE,"started");

    if ( driver_open ( &driver, S710_MODE_RDONLY ) < 0 ) {
      syslog(LOG_CRIT,"failed to initialize, aborting");
      exit(1);
    }

    /* install signal handlers */

    signal(SIGINT,signal_handler);
    signal(SIGTERM,signal_handler);

    /* enter infinite loop */
    
    while ( !gSigFlag ) {

      if ( receive_file(&driver,&file,stderr) ) {
	save_files(&file,filedir,stderr);
	print_files(&file,stderr); 
      }      

      usleep(10000);
    }

    driver_close ( &driver );

    /* note the signal */

    syslog(LOG_NOTICE,"ended: received signal %d",gSigFlag);
    closelog();   /* optional */
  }

  return 0;
}



/* signal handler just sets a global flag */

static void
signal_handler ( int signum )
{
  gSigFlag = signum;
}
