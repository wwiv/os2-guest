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
#include "config.h"
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

int show_help() {
  printf(
    "Usage: \r\n"
    "  vmtools [args]\r\n\r\n"
    "Example: \r\n"
    "  vmtools\r\n\r\n"
    "Commands: \r\n"
    "  -Q             Terminates daemon if any exists.\r\n"
    "  -?             Help - displays this information\r\n"
    "\r\n");
  return 0;
}

int exit_app() {
  char* shm;
  APIRET rc = DosGetNamedSharedMem((PVOID *) &shm,
				   NAME_SEG,
				   PAG_READ|PAG_WRITE);
  if (rc != NO_ERROR) {
    logf(0, "Failed to connect to SHM: %d\r\n", rc);
    return 1;
  }
  log(0, "Sending quit signal.\r\n");

  *shm = 'Q';
  DosSleep(1000);
  DosFreeMem(shm);
  return 0;
}

int main(int argc, char* argv[]) {
  printf("vmtools: OS/2 Guest for VMWare %s; [https://github.com/wwiv/os2-guest]\r\n\r\n", 
	 VMTOOLS_VERSION);

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
      case 'Q':
	return exit_app();
      case '?':
        return show_help();
      }
      continue;
    }
  } 
  return show_help();
}


