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
#include <ctype.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

#define INCL_KBD
#define INCL_PM
#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_DOSPROCESS
#define INCL_DOSMEMMGR
#define INCL_ERRORS

#include <os2.h>

#define NUMBER_OF_BYTES 255
#define NAME_SEG "\\SHAREMEM\\VMTOOLS.MEM"

int vmtools_daemon() {
  printf("Starting Daemon...\r\n");
  char* shm = NULL;
  APIRET rc = DosAllocSharedMem((PVOID *) &shm,
				NAME_SEG,
				NUMBER_OF_BYTES,
				PAG_READ | PAG_WRITE | PAG_COMMIT);
  if (rc != NO_ERROR) {
    // Unable to allocate shared memory
    printf("Failed to allocate shared memory: %d\r\n", rc);
    return 1;
  }
  

  fprintf(stdout, "Press ESCAPE to exit.\r\n");
  KBDKEYINFO key;
  Host host;
  Guest guest;
  bool pointer_host = pointer_in_host(host.pointer());
  bool mouse_hidden = false;

  for (;;) {
    KbdPeek(&key, 0);
    if (key.fbStatus != 0 && key.chChar == 27 /* ESC */) {
      puts("Exiting...\r\n");
      break;
    }

    host_point hp = host.pointer();
    //printf("pointer in host[%d], pointer_host[%d,%d]\r\n", 
    //   pointer_in_host(hp), hp.x, hp.y);
    if (pointer_in_host(hp) && !pointer_host) {
      puts("pointer back in host");
      // Moved to the host
      pointer_host = true;
      char* c = guest.clipboard();
      if (c) {
	//printf("Set host clipboard: [%s]\r\n", c);
	host.clipboard(c);
      }
      // Hide mouse if it's visible.
      if (!mouse_hidden) {
	guest.pointer_visible(false);
        mouse_hidden = true;
      }
    } else if (!pointer_in_host(hp) && pointer_host) {
      puts("pointer back in guest\r\n");
      pointer_host = false;
      char* c = host.clipboard();
      if (c) {
	//printf("Set guest clipboard: [%s]\r\n", c);
	guest.clipboard(c);
      }
      // Set the guest pointer to match.
      guest_point gp = guest.host_to_guest(hp);
      guest.pointer(gp);
      if (mouse_hidden) {
	guest.pointer_visible(true);
	mouse_hidden = false;
      }
    }

    if (!pointer_host) {
      // Grab the current position, flip it, and send to the host.
      guest_point gp = guest.pointer();
      host_point hp = guest.guest_to_host(gp);
      host.pointer(hp);
    }

    // TODO check for shutdown signal if running as a daemon.
    if (shm != NULL)  {
      if (*shm == 'Q') {
	// time to quit.
	printf("Got signal to quit.\r\n");
	break;
      }
    } 
    DosSleep(100);
  }

  DosFreeMem(shm);
  return 0;
}

int show_help() {
  fprintf(stderr,
    "Usage: \r\n"
    "  vmtools [args]\r\n\r\n"
    "Example: \r\n"
    "  vmtools\r\n\r\n"
    "Commands: \r\n"
    "  -Q    Terminates daemon if any exists.\r\n"
    "  -?    Help - displays this information\r\n"
    "\r\n");
  return 0;
}

int exit_app() {
  char* shm;
  APIRET rc = DosGetNamedSharedMem((PVOID *) &shm,
				   NAME_SEG,
				   PAG_READ|PAG_WRITE);
  if (rc != NO_ERROR) {
    printf("Failed to connect to SHM: %d\r\n", rc);
    return 1;
  }
  printf("Sending quit signal.\r\n");

  *shm = 'Q';
  DosSleep(1000);
  DosFreeMem(shm);
  return 0;
}

int main(int argc, char* argv[]) {
  fprintf(stdout, "VMTools: OS2 Guest for VMWare.\r\n"
	  "See https://github.com/wwiv/os2-guest for more information.\r\n\r\n"
	  "This program uses portions of [https://github.com/vmware/open-vm-tools]\r\n"
	  "which are licensed under the GNU Lesser General Public Library, version 2.1\r\n");

  if (argc <= 1) {
    return vmtools_daemon();
  }

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
      // Process switch
      char schar = (char)toupper(*(arg+1));
      const char* sval = (arg+2);
      switch (schar) {
      case 'Q':
	return exit_app();
      case '?':
        return show_help();
      }
      continue;
    }
  } 

  fprintf(stdout, "exiting.");
  return 0;
}

