/*                                                                      
 *   OS/2 Guest Tools for VMWare
 *   Copyright (C)2021, Rushfan
 *
 *   Licensed  under the  Apache License, Version  2.0 (the "License");
 *   you may not use this  file  except in compliance with the License.
 *   You may obtain a copy of the License at                          
 *                                                                     
 *               http://www.apache.org/licenses/LICENSE-2.0           
 *                                                                     
 *   Unless  required  by  applicable  law  or agreed to  in  writing,
 *   software  distributed  under  the  License  is  distributed on an
 *   "AS IS"  BASIS, WITHOUT  WARRANTIES  OR  CONDITIONS OF ANY  KIND,
 *   either  express  or implied.  See  the  License for  the specific
 *   language governing permissions and limitations under the License.
 */
#include "guest.h"
#include "host.h"
#include "log.h"

#include <ctype.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#define INCL_KBD
#define INCL_PM
#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_DOSPROCESS
#define INCL_DOSMEMMGR
#define INCL_ERRORS

#include <os2.h>

// shared memory layout
// byte 0: command
// byte 1-15: command args
// byte 16-255: response from command
// commands:
// Q : quit
// S : Status (display stats in 16+)
// R : Reply  (Server responded)

#define NUMBER_OF_BYTES 255
#define NAME_SEG "\\SHAREMEM\\VMTOOLS.MEM"

int vmtools_daemon() {
  LOG_FUNCTION();
  printf("Press ESCAPE to exit.\r\nStarting Daemon...\r\n");
  char* shm = NULL;
  APIRET rc = DosAllocSharedMem((PVOID *) &shm,
				NAME_SEG,
				NUMBER_OF_BYTES,
				PAG_READ | PAG_WRITE | PAG_COMMIT);
  if (rc != NO_ERROR) {
    // Unable to allocate shared memory
    logf(0, "Failed to allocate shared memory: %d\r\n", rc);
    return 1;
  }

  memset(shm, 0, NUMBER_OF_BYTES);

  KBDKEYINFO key;
  Host host;
  Guest guest;
  host_point hp = host.pointer();
  bool pointer_host = pointer_in_host(hp);
  bool mouse_hidden = false;

  for (;;) {
    KbdPeek(&key, 0);
    if (key.fbStatus != 0) {
      APIRET krc = KbdCharIn(&key, IO_WAIT, 0);
      logf(4, "got key: [%d]\r\n", key.chChar);
      if (key.chChar == 27 /* ESC */) {
	puts("Exiting...\r\n");
	break;
      }
    }

    hp = host.pointer();
    logf(5, "pointer in host[%d], pointer_host[%d,%d]", 
	 pointer_in_host(hp), hp.x, hp.y);
    if (pointer_in_host(hp) && !pointer_host) {
      log(1, "pointer back in host");
      // Moved to the host
      pointer_host = true;
      char* c = guest.clipboard();
      if (c) {
	logf(1, "Set host clipboard: [%s]", c);
	host.clipboard(c);
      }
      // Hide mouse if it's visible.
      if (!mouse_hidden) {
	log(1, "mouse hidden\r\n");
	guest.pointer_visible(false);
        mouse_hidden = true;
      }
    } else if (!pointer_in_host(hp) && pointer_host) {
      log(1, "pointer back in guest");
      pointer_host = false;
      if (mouse_hidden) {
	guest.pointer_visible(true);
	mouse_hidden = false;
      }
      char* c = host.clipboard();
      if (c) {
	logf(3, "Set guest clipboard: [%s]\r\n", c);
	guest.clipboard(c);
      }

      // Set the guest pointer to match.
      guest_point ogp = guest.pointer();
      guest_point gp = guest.host_to_guest(hp);
      guest.pointer(gp);
      logf(1, "[guest] pointer moved from [%d,%d] to [%d,%d]\r\n", ogp.x, ogp.y, gp.x, gp.y);

    } else if (!pointer_host) {
      // Grab the current position, flip it, and send to the host.
      guest_point gp = guest.pointer();
      host_point hp = guest.guest_to_host(gp);
      host.pointer(hp);
      logf(4, "[guest]pointer[%d,%d]\r\n", gp.x, gp.y);
    }

    // TODO check for shutdown signal if running as a daemon.
    if (shm != NULL)  {
      if (*shm == 'Q') {
	// time to quit.
	log(0, "Got signal to quit.\r\n");
	break;
      }
    }
    DosSleep(100);
  }

  if (mouse_hidden) {
    guest.pointer_visible(true);
  }
  DosFreeMem(shm);
  return 0;
}

int main(int argc, char* argv[]) {
  printf("vmtools: OS/2 Guest for VMWare. [https://github.com/wwiv/os2-guest]\r\n\r\n");

  // Interactive mode

  for (int i = 1; i < argc; i++) {
    const char* arg = argv[i];
    const int alen = strlen(arg);
    if (!alen) {
      continue;
    }
    if (arg[0] == '-' || arg[0] == '/') {
      if (alen < 2) {
	continue;
      }
      // Process switch.  If this should not go to a daemon
      // then exit while processing the command.
      char schar = (char)toupper(*(arg+1));
      const char* sval = (arg+2);
      switch (schar) {
      case 'D': {
	if (sval) {
	  set_loglevel(atoi(sval));
	}
      } break;
      case 'L': {
	if (sval) {
	  set_logfile(sval);
	}
      } break;
      }
      continue;
    }
  } 

  return vmtools_daemon();
}


