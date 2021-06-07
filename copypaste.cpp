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
#include <thread>

static constexpr int XPOS_IN_HOST = -100;
static std::atomic<bool> stop_copy_thread;
static std::thread copy_thread;

enum class pointer_home_t { host, guest };
static std::string contents;

static HAB hab;
static HMQ hmq;

static void log(const char* s) {
  fprintf(stderr, "%s\r\n", s);
  fflush(stderr);
}

static void PointerInHost() {
  fprintf(stderr, "PointerInHost\r\n");
  fflush(stderr);
}

static void PointerInGuest() {
  fprintf(stderr, "PointerInGuest\r\n");
  fflush(stderr);
}

void GetClipboardFromContents() {
  log(__PRETTY_FUNCTION__);
  if (WinOpenClipbrd(hab)) {
    log("WinOpenClipbrd: succeed");
    ULONG fmtInfo = 0;
    if (WinQueryClipbrdFmtInfo(hab, CF_TEXT, &fmtInfo)) {
      log("Has text in clipboard");
      char *text = (char*)WinQueryClipbrdData(hab, CF_TEXT);
      log("after query clipboard");
      if (text) {
	log("after text check");
	int len = strlen(text);
	log("after strlen(text)");
	fprintf(stderr, "Got Text: len: %d", len); fflush(stderr);
	contents.assign(text);
	log("contents assigned");
      }
    }
    log("Closing Clipboard");
    WinCloseClipbrd(hab);
  }
}

static void SetClipboardToContents(const std::string& c) {
  log(__PRETTY_FUNCTION__);
  if (WinOpenClipbrd(hab)) {
    WinEmptyClipbrd(hab);

    char* clipboard_data;
    auto rc = DosAllocSharedMem((PPVOID)&clipboard_data,
                NULL, c.size()+1, PAG_COMMIT | PAG_WRITE | OBJ_GIVEABLE);
    if (rc == NO_ERROR) {
      strcpy(clipboard_data, c.c_str());
      fprintf(stderr, "Set Clipboard: '%s'", c.c_str());
      WinSetClipbrdData(hab, (ULONG) clipboard_data, CF_TEXT, CFI_POINTER);
    } else {
      log("Failed to alloc shared memory for clipboard");
    }
    WinCloseClipbrd(hab);
  } else {
    log("Failed to open Clipboard");
  }
}

static void SendClipboardToHost() {
  log(__PRETTY_FUNCTION__);
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

  log("1 :CopyPasteHelperThread");
  char buf[1024 + 4];
  int16_t host_x, host_y;
  pointer_home_t pointer_home{pointer_home_t::host};

  // This is what PD curses does before calling WinInitalize.  type 3
  // seems to be a PM App.
  DosGetInfoBlocks(&ptib, &ppib);
  ppib->pib_ultype = 3;
  hab = WinInitialize(0L);
  hmq = WinCreateMsgQueue(hab, 0);

  fprintf(stderr, "HAB: %d\r\nHMQ: %d\r\n", hab, hmq); fflush(stderr);

  for (;;) {
    PointerGetPos(&host_x, &host_y);
    // Handle moving between host and guest.
    if (host_x == XPOS_IN_HOST && pointer_home != pointer_home_t::host) {
      // Moved to the host
      pointer_home = pointer_home_t::host;
      PointerInHost();
      GetClipboardFromContents();
      SendClipboardToHost();
      // contents is now the clipboard

    } else if (host_x != XPOS_IN_HOST && pointer_home != pointer_home_t::guest) {
      // Back to the guest
      pointer_home = pointer_home_t::guest;
      PointerInGuest();
      if (int32_t hclen = CopyPaste_GetHostSelectionLen(); hclen > 0 && hclen <= 8196) {
        log("Got Clipboard from host");
	contents.resize(hclen + 21);
	CopyPaste_GetHostSelection(hclen, &contents[0]);
	contents.resize(hclen);
	SetClipboardToContents(contents);
      }
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
  log(__PRETTY_FUNCTION__);
  stop_copy_thread.store(false);
  std::thread t(CopyPasteHelperThread);
  std::swap(copy_thread, t);
}

void StopCopyPasteThread() {
  log(__PRETTY_FUNCTION__);
  stop_copy_thread.store(true);
  DosSleep(10);
  if (copy_thread.joinable()) {
    copy_thread.join();
  }
}






