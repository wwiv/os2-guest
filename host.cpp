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
#include "host.h"
#define INCL_DOSPROCESS
#define INCL_ERRORS
#include <os2.h>


#include "backdoor.h"

// Use stdint.h vs. cstdint so types are in global namespace, not std::
#include <stdint.h>
#include <string.h>
#include <iostream>


/* From https://sites.google.com/site/chitchatvmback/backdoor
   =========================================================================================
   02h	APM function	                Y	Y	Y	Y *1	Y *1	Y	Y *1
   04h	Get mouse cursor position	Y	Y	Y	Y	Y	Y	Y
   05h	Set mouse cursor position	Y	Y	Y	Y	Y	Y	Y
   06h	Get text length from clipboard	Y	Y	Y	Y	Y	Y	Y
   07h	Get text from clipboard	        Y	Y	Y	Y	Y	Y	Y
   08h	Set text length to clipboard	Y	Y	Y	Y	Y	Y	Y
   09h	Set text to clipboard	        Y	Y	Y	Y	Y	Y	Y
   =========================================================================================
   ECX gets command
   EBX gets optional params
*/

#define BACKDOOR_CMD_GET_MOUSE_POS 0x04
#define BACKDOOR_CMD_SET_MOUSE_POS 0x05
#define BACKDOOR_CMD_GET_CLIPBOARD_LEN 0x06
#define BACKDOOR_CMD_GET_CLIPBOARD_TEXT 0x07
#define BACKDOOR_CMD_SET_CLIPBOARD_LEN 0x08
#define BACKDOOR_CMD_SET_CLIPBOARD_TEXT 0x09


/** 
 * Gets the host clipboard length in bytes. 
 * returns <0 on error
 */
int32_t backdoor_get_clipbopard_len() {
  return Backdoor(BACKDOOR_CMD_GET_CLIPBOARD_LEN);
}

/** 
 * Gets the next 4 bytes of the clipboard or 0 if none are available.
 * returns <0 on error
 */
int32_t backdoor_get_host_clipbopard_text_piece() {
  return Backdoor(BACKDOOR_CMD_GET_CLIPBOARD_TEXT);
}

void get_host_clipboard(int32_t size, char* buf) {
   uint32_t *current = (uint32_t *)buf;
   uint32_t const *end = current + (size + sizeof(uint32_t) - 1) / sizeof(uint32_t);
   for (; current < end; current++) {
      *current = backdoor_get_host_clipbopard_text_piece();
   }
}

int32_t set_host_clipboard_len(uint32_t len) {
  return Backdoor2(BACKDOOR_CMD_SET_CLIPBOARD_LEN, len);
}

void set_host_clipboard_text_piece(uint32_t data) {
  Backdoor2(BACKDOOR_CMD_SET_CLIPBOARD_TEXT, data);
}

void set_host_clipboard(char* buf, uint32_t len) {
  set_host_clipboard_len(len);
  uint32_t* p = (uint32_t*) buf;
  for (int i = 0; i < len; i += sizeof(uint32_t)) {
    set_host_clipboard_text_piece(*p++);
  }
}

static void set_mouse_pos(int x, int y) {
  uint32_t pos = (x << 16) | y;
  Backdoor2(BACKDOOR_CMD_SET_MOUSE_POS, pos);
}

static bool get_mouse_pos(int* x, int* y) {
  uint32_t pos = Backdoor(BACKDOOR_CMD_GET_MOUSE_POS);
  *x = pos >> 16;
  *y = pos & 0xFFFF;

  return true;
}

Host::Host() {}
Host::~Host() {}

host_point Host::pointer() {
  host_point pos;
  get_mouse_pos(&pos.x, &pos.y);
  return pos;
}

bool Host::pointer(const host_point& pos) {
  set_mouse_pos(pos.x, pos.y);
  return true;
}

bool Host::clipboard(char* b) {
  if (!b) {
    return false;
  }
  int len = strlen(b);
  set_host_clipboard(b, len);
  return true;
}

char* Host::clipboard() {
  const int32_t len = backdoor_get_clipbopard_len();
  if (len <= 0) {
    std::cout << "alloc failed" << std::endl;
    return NULL;
  }
  std::cout << "len: " << len << std::endl;
  char* b;
  APIRET rc = DosAllocSharedMem((PPVOID)&b,
			     NULL, len+1, 
			     PAG_COMMIT | PAG_WRITE | OBJ_GIVEABLE);
  if (rc != NO_ERROR) {
    std::cout << "alloc failed" << std::endl;
    return false;
  }
  get_host_clipboard(len, b);
  return b;
}


int main(int argc, char* argv) {
  Host b;
  char* clip =  b.clipboard();
  if (clip) {
    std::cout << "Clip: " << clip << std::endl;
    DosFreeMem(clip);
  } else {
    std::cout << "[no clipboard]" << std::endl;
  }

  host_point pos;
  for (;;) {
    host_point pos = b.pointer();
    std::cout << "pos: " << pos.x 
	      << ", " << pos.y << std::endl;
    DosSleep(500);
  }
  std::cout << "end\r\n";
}




