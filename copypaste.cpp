/*                                                                      
 *   OS/2 Guest Tools for VMWare
 *   Copyright (C)1998-2021, Rushfan
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
#include "copyPasteCompat.h"
#include "ovt/pointer.h"

#define INCL_PM
#define INCL_DOSERRORS
#define INCL_DOSPROCESS
#include <os2.h>

#include <algorithm>
#include <atomic>
#include <cstdio>
#include <cstring>
#include <string>
#include <string_view>
#include <thread>

// Logs at a higheer level than threshold will not be logged.
static constexpr int LOG_THRESHOLD = 1;

static constexpr int XPOS_IN_HOST = -100;
static std::atomic<bool> stop_copy_thread;
static std::thread copy_thread;

enum class pointer_home_t { host, guest };

static HAB hab;
static HMQ hmq;

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

std::string GetClipboardFromContents() {
  log(2,__PRETTY_FUNCTION__);
  if (WinOpenClipbrd(hab)) {
    log(3,"WinOpenClipbrd: succeed");
    ULONG fmtInfo = 0;
    if (WinQueryClipbrdFmtInfo(hab, CF_TEXT, &fmtInfo)) {
      log(4, "Has text in clipboard");
      if (const char *text = (const char*)WinQueryClipbrdData(hab, CF_TEXT); text != nullptr) {
	const int len = strlen(text);
	std::string c(text);
	WinCloseClipbrd(hab);
	return c;
	log(4, "contents assigned");
      }
    }
    log(4, "Closing Clipboard");
    WinCloseClipbrd(hab);
  } else {
    log("Failed to open Clipboard");
  }
  return{};
}

static void SetClipboardData(const char* clipboard_data) {
  log(2, __PRETTY_FUNCTION__);
  if (WinOpenClipbrd(hab)) {
    WinEmptyClipbrd(hab);
    WinSetClipbrdData(hab, (ULONG) clipboard_data, CF_TEXT, CFI_POINTER);
    WinCloseClipbrd(hab);
  } else {
    log("Failed to open Clipboard");
    DosFreeMem((PVOID) clipboard_data);
  }
}

static void SendClipboardToHost(const std::string& contents) {
  log(2, __PRETTY_FUNCTION__);
  uint32_t* p = (uint32_t*) contents.c_str();
  CopyPaste_SetSelLength(contents.size());
  for (int i = 0; i < contents.size(); i += sizeof(uint32_t)) {
    CopyPaste_SetNextPiece(*p);
    ++p;
  }
}

static void CopyPasteHelperThread() {
  PTIB ptib;
  PPIB ppib;

  log(2, __PRETTY_FUNCTION__);
  char buf[1024 + 4];
  int16_t host_x, host_y;
  pointer_home_t pointer_home{pointer_home_t::host};

  // This is what PD curses does before calling WinInitalize.  type 3
  // seems to be a PM App.
  DosGetInfoBlocks(&ptib, &ppib);
  ppib->pib_ultype = 3;
  hab = WinInitialize(0L);
  hmq = WinCreateMsgQueue(hab, 0);

  for (;;) {
    PointerGetPos(&host_x, &host_y);
    // Handle moving between host and guest.
    if (host_x == XPOS_IN_HOST && pointer_home != pointer_home_t::host) {
      // Moved to the host
      pointer_home = pointer_home_t::host;
      auto c = GetClipboardFromContents();
      SendClipboardToHost(c);
    } else if (host_x != XPOS_IN_HOST && pointer_home != pointer_home_t::guest) {
      // Back to the guest
      pointer_home = pointer_home_t::guest;
      if (int32_t hclen = CopyPaste_GetHostSelectionLen(); hclen > 0 && hclen <= 8196) {
        log(2, "Got Clipboard from host");
	char* clipboard_data;
	if (auto rc = DosAllocSharedMem((PPVOID)&clipboard_data,
				    NULL, hclen+1, PAG_COMMIT | PAG_WRITE | OBJ_GIVEABLE); rc == NO_ERROR) {
	  CopyPaste_GetHostSelection(hclen, clipboard_data);
	  SetClipboardData(clipboard_data);
	}
      }

      // Fetch the max screen size for Y since OS/2's coordinates are cartesian
      // (aka bottom right origin) And most others are not (top left origin)
      // Set the guest pointer to match.
      const auto y =  WinQuerySysValue(HWND_DESKTOP, SV_CYSCREEN) - host_y;
      WinSetPointerPos(HWND_DESKTOP, host_x, y);

    }

    if (stop_copy_thread.load() == true) {
      log("signal received to end copy thread.");
      break;
    }

    // TODO - update guest location based on host position?
    DosSleep(200);
  }

  // Cleanup
  WinDestroyMsgQueue(hmq);
  WinTerminate(hab);
}

void StartCopyPasteThread() {
  log(2, __PRETTY_FUNCTION__);
  stop_copy_thread.store(false);
  std::thread t(CopyPasteHelperThread);
  std::swap(copy_thread, t);
}

void StopCopyPasteThread() {
  log(2, __PRETTY_FUNCTION__);
  stop_copy_thread.store(true);
  DosSleep(10);
  if (copy_thread.joinable()) {
    copy_thread.join();
  }
}







