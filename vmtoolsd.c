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
#include "vmtypes.h"

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
#define NAME_PIPE "\\PIPE\\VMTOOLSD"

static bool pm_mode = true;

typedef struct {
  HPIPE handle;
  HEV hev;
} pipe_t;

bool create_pipe(pipe_t* pipe) {
  return true;
}

void gopm() {
  PTIB ptib;
  PPIB ppib;

  DosGetInfoBlocks(&ptib, &ppib);
  ppib->pib_ultype = PT_PM;
}

int vmtools_daemon() {
  char* shm = NULL;
  APIRET rc = NO_ERROR;
  guest_info guest;
  host_point hp;
  bool mouse_hidden = false;
  bool pointer_host = false;

  LOG_FUNCTION();
  printf("Starting Daemon...\r\n");

  if (pm_mode) {
    gopm();
  }
  guest_init(&guest);
  rc = DosAllocSharedMem((PVOID *) &shm,
			 NAME_SEG,
			 NUMBER_OF_BYTES,
			 PAG_READ | PAG_WRITE | PAG_COMMIT);
  if (rc != NO_ERROR) {
    // Unable to allocate shared memory
    loglf(0, "Failed to allocate shared memory: %d\r\n", rc);
    return 1;
  }

  memset(shm, 0, NUMBER_OF_BYTES);

  if (!get_host_pointer(&hp)) {
      loglf(1, "Failed to get host pointer");
      return 1;
  }
  pointer_host = pointer_in_host(&hp);

  for (;;) {
    
    if (!get_host_pointer(&hp)) { 
      // error
      loglf(1, "Failed to get host pointer");
      break;
    }
    loglf(5, "pointer in host[%d], pointer_host[%d,%d]", 
	 pointer_in_host(&hp), hp.x, hp.y);
    if (pointer_in_host(&hp) && !pointer_host) {
      char* c = NULL;
      logl(1, "pointer back in host");
      // Moved to the host
      pointer_host = true;
      c = get_guest_clipboard(&guest);
      if (c) {
	loglf(1, "set host clipboard: [%s]", c);
	set_host_clipboard(c);
      }
      // Hide mouse if it's visible.
      if (!mouse_hidden) {
	logl(1, "mouse hidden\r\n");
	set_guest_pointer_visible(&guest, false);
        mouse_hidden = true;
      }
    } else if (!pointer_in_host(&hp) && pointer_host) {
      logl(1, "pointer back in guest");
      pointer_host = false;
      if (mouse_hidden) {
	set_guest_pointer_visible(&guest, true);
	mouse_hidden = false;
      }
      {
	char* c = get_host_clipboard();
	if (c) {
	  loglf(3, "Set guest clipboard: [%s]\r\n", c);
	  set_guest_clipboard(&guest, c);
	}
      }
      // Set the guest pointer to match.
      {
	guest_point ogp;
	guest_point gp;
	get_guest_pointer(&guest, &ogp);
	host_to_guest(&guest, &hp, &gp);
	set_guest_pointer(&guest, &gp);
	loglf(1, "[guest] pointer moved from [%d,%d] to [%d,%d]\r\n", ogp.x, ogp.y, gp.x, gp.y);
      }

    } else if (!pointer_host) {
      // Grab the current position, flip it, and send to the host.
      guest_point gp;
      host_point hp;

      get_guest_pointer(&guest, &gp);
      guest_to_host(&guest, &gp, &hp);
      set_host_pointer(&hp);
      loglf(4, "[guest]pointer[%d,%d]\r\n", gp.x, gp.y);
    }

    // TODO check for shutdown signal if running as a daemon.
    if (shm != NULL)  {
      if (*shm == 'Q') {
	// time to quit.
	logl(0, "Got signal to quit.\r\n");
	break;
      }
    }
    DosSleep(100);
  }

  if (mouse_hidden) {
    set_guest_pointer_visible(&guest, true);
  }

  guest_destroy(&guest);
  DosFreeMem(shm);

  return 0;
}

int main(int argc, char* argv[]) {
  int i;
  printf("vmtools: OS/2 Guest for VMWare. [https://github.com/wwiv/os2-guest]\r\n\r\n");

  // Interactive mode

  for (i = 1; i < argc; i++) {
    char* arg = argv[i];
    const int alen = strlen(arg);
    if (!alen) {
      continue;
    }
    if (arg[0] == '-' || arg[0] == '/') {
      char schar = 0;
      char* sval = NULL;
      if (alen < 2) {
	continue;
      }
      // Process switch.  If this should not go to a daemon
      // then exit while processing the command.
      schar = (char)toupper(*(arg+1));
      sval = (arg+2);
      switch (schar) {
      case 'C': {
	pm_mode = false;
      } break;
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


