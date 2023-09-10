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

#include "log.h"

// Use stdint.h vs. cstdint so types are in global namespace, not std::
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define INCL_PM
#define INCL_DOS
#define INCL_ERRORS
#define INCL_DOSPROCESS
#include <os2.h>

bool guest_init(guest_info* guest) {
  APIRET rc = NO_ERROR;
  memset(guest, 0, sizeof(guest_info));

  guest->last_point_.x = -1;
  guest->last_point_.y = -1;

  // TODO(rushfan): Check for errors here.
  guest->hab_ = WinInitialize(0L);
  if (!guest->hab_) {
    logl(0, "Failed to create HAB\r\n");
  }
  guest->hmq_ = WinCreateMsgQueue(guest->hab_, 0);
  if (!guest->hmq_) {
    logl(0, "Failed to create HMQ\r\n");
  }

  guest->screen_max_y_ = WinQuerySysValue(HWND_DESKTOP, SV_CYSCREEN);
  loglf(2, "Screen size max y: [%d]\r\n", guest->screen_max_y_);

  // Lower priority to use less CPU
  rc = DosSetPriority(PRTYS_PROCESS, 
		      PRTYC_IDLETIME, 
		      0, /* no delta */
		      0 /* current process */ );
  if (rc != NO_ERROR) {
    loglf(0, "Error setting priority, rc=%d", rc);
  }
  return true;
}

bool guest_destroy(guest_info* guest) {
  // Cleanup
  WinDestroyMsgQueue(guest->hmq_);
  WinTerminate(guest->hab_);
  return true;
}

  
/** Gets the guest pointer position */
bool get_guest_pointer(guest_info* guest, guest_point* guest_pos) {
  POINTL pos;
  WinQueryPointerPos(HWND_DESKTOP, &pos);

  guest_pos->x = pos.x;
  guest_pos->y = pos.y;

  if (guest->last_point_.x != pos.x && guest->last_point_.y != pos.y) {
    loglf(1, "new pos: [%d, %d]; old: [%d,%d]", pos.x, pos.y, guest->last_point_.x, guest->last_point_.y);
    guest->last_point_.x = pos.x;
    guest->last_point_.y = pos.y;
  } else {
    loglf(4, "remaining at last point: [%d, %d]", pos.x, pos.y);
  }

  return true;
}


/** Sets the guest pointer position */
bool set_guest_pointer(guest_info* guest, const guest_point* pos) {
  if (!WinSetPointerPos(HWND_DESKTOP, pos->x, pos->y)) {
    loglf(0, "failed to set pointer pos: %d", WinGetLastError(guest->hab_));
  }
  if (guest->last_point_.x != pos->x && guest->last_point_.y != pos->y) {
    loglf(1, "new pos: [%d, %d]; old: [%d,%d]", pos->x, pos->y, guest->last_point_.x, guest->last_point_.y);
    guest->last_point_.x = pos->x;
    guest->last_point_.y = pos->y;
  } else {
    loglf(4, "remaining at last point: [%d, %d]", pos->x, pos->y);
  }
  return true;
}

bool set_guest_pointer_visible(guest_info* guest, bool visible) {
  loglf(1, "Guest::pointer_visible(%s)", visible ? "true": "false");
  WinShowPointer(HWND_DESKTOP, visible ? TRUE : FALSE);
  return true;
}

/** Sets the guest clipboard contents or releases b if that fails. */
bool set_guest_clipboard(guest_info* guest, char* b) {
  if (WinOpenClipbrd(guest->hab_)) {
    logl(3, "opened clipboard");
    WinEmptyClipbrd(guest->hab_);
    WinSetClipbrdData(guest->hab_, (ULONG) b, CF_TEXT, CFI_POINTER);
    WinCloseClipbrd(guest->hab_);
    return true;
  }
  logl(0, "Failed to open Clipboard");
  DosFreeMem((PVOID) b);
  return false;
}


/** Gets the guest clipboard contents or NULL if none exist */
char* get_guest_clipboard(guest_info* guest) {
  char* ret = NULL;
  ULONG fmtInfo = 0;
  LOG_FUNCTION();

  if (!WinOpenClipbrd(guest->hab_)) {
    logl(2, "Failed to open Clipboard");
    return NULL;
  }
  
  logl(2, "WinOpenClipbrd: succeed");
  if (WinQueryClipbrdFmtInfo(guest->hab_, CF_TEXT, &fmtInfo)) {
    const char *text = (const char*)WinQueryClipbrdData(guest->hab_, CF_TEXT); 
    logl(2, "WinQueryClipbrdFmtInfo: succeed");
    if (text && *text) {
      loglf(3, "Has text in clipboard: [%s]", text);
      ret = strdup(text);
      logl(1, "contents assigned");
    } else {
      logl(1, "it was empty");
    }
  }

  logl(4, "Closing Clipboard");
  WinCloseClipbrd(guest->hab_);

  return ret;
}

void host_to_guest(guest_info* guest, const host_point* hp, guest_point* r) {
  r->x = hp->x;
  r->y = guest->screen_max_y_ - hp->y;
}

void guest_to_host(guest_info* guest, const guest_point* gp, host_point* r) {
  r->x = gp->x;
  r->y = guest->screen_max_y_ - gp->y;
}



