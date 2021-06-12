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

// Use stdint.h vs. cstdint so types are in global namespace, not std::
#include <stdio.h>
#include <stdint.h>
#include <cstdlib>
#include <cstring>

#define INCL_PM
#define INCL_DOS
#define INCL_ERRORS
#include <os2.h>

// Logs at a higheer level than threshold will not be logged.
static const int LOG_THRESHOLD = 1;
static void log(const char* s) {
  fprintf(stderr, "%s\r\n", s);
  fflush(stderr);
}

/**
 * Log at log level lvl. 
 * higher levels mean more logging.  Use 1-4.
 */
static void log(int lvl, const char* s) {
  if (lvl < LOG_THRESHOLD) {
    log(s);
  }
}


Guest::Guest() {
  PTIB ptib;
  PPIB ppib;
  DosGetInfoBlocks(&ptib, &ppib);
  ppib->pib_ultype = 3;

  // TODO(rushfan): Check for errors here.
  hab_ = WinInitialize(0L);
  hmq_ = WinCreateMsgQueue(hab_, 0);

  screen_max_y_ = WinQuerySysValue(HWND_DESKTOP, SV_CYSCREEN);
}

Guest::~Guest() {
  // Cleanup
  WinDestroyMsgQueue(hmq_);
  WinTerminate(hab_);
}

  
/** Gets the guest pointer position */
guest_point Guest::pointer() {
  POINTL pos;
  WinQueryPointerPos(HWND_DESKTOP, &pos);

  guest_point guest_pos;
  guest_pos.x = pos.x;
  guest_pos.y = pos.y;
  return guest_pos;
}


/** Sets the guest pointer position */
bool Guest::pointer(const guest_point& pos) {
  WinSetPointerPos(HWND_DESKTOP, pos.x, pos.y);
  return true;
}

bool Guest::pointer_visible(bool visible) {
  WinShowPointer(HWND_DESKTOP, visible ? TRUE : FALSE);
  return true;
}


/** Sets the guest clipboard contents or releases b if that fails. */
bool Guest::clipboard(char* b) {
  log(1, __FUNCTION__);
  if (WinOpenClipbrd(hab_)) {
    log(1, "opened clipboard");
    WinEmptyClipbrd(hab_);
    WinSetClipbrdData(hab_, (ULONG) b, CF_TEXT, CFI_POINTER);
    WinCloseClipbrd(hab_);
    return true;
  }
  log("Failed to open Clipboard");
  DosFreeMem((PVOID) b);
  return false;
}


/** Gets the guest clipboard contents or NULL if none exist */
char* Guest::clipboard() {
  log(1, __FUNCTION__);
  if (!WinOpenClipbrd(hab_)) {
    log("Failed to open Clipboard");
    return NULL;
  }
  
  char* ret = NULL;
  log(2, "WinOpenClipbrd: succeed");
  ULONG fmtInfo = 0;
  if (WinQueryClipbrdFmtInfo(hab_, CF_TEXT, &fmtInfo)) {
    log(3, "Has text in clipboard");
    const char *text = (const char*)WinQueryClipbrdData(hab_, CF_TEXT); 
    if (text) {
      ret = strdup(text);
      log(1, "contents assigned");
    }
  }
  log(4, "Closing Clipboard");
  WinCloseClipbrd(hab_);
  return ret;
}

guest_point Guest::host_to_guest(const host_point& hp) {
  guest_point r;
  r.x = hp.x;
  r.y = screen_max_y_ - hp.y;
  
  return r;
}

host_point Guest::guest_to_host(const guest_point& gp) {
  host_point r;
  r.x = gp.x;
  r.y = screen_max_y_ - gp.y;

  return r;
}



