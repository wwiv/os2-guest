/*                                                                      
 *   OS/2 Guest Tools for VMWare
 *   Copyright (C)2023, Rushfan
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
#include "backdoor.h"
#include "host.h"
#include "log.h"
#include "vmtypes.h"


static const uint32_t _BDOOR_MAGIC = 0x564D5868;
static const uint16_t _BDOOR_PORT  = 0x5658;

static inline int _backdoorasm(int _a, int _b) {
  int _ret = -1;
  _asm {
    mov ecx, _a
    mov ebx, _b
    mov eax, _BDOOR_MAGIC
    mov dx, _BDOOR_PORT
    in eax, dx
    mov _ret, eax
  };
  return _ret;
}

int Backdoor(int a) {
  return _backdoorasm(a, 0);
}

int Backdoor2(int a, int b) {
  return _backdoorasm(a, b);
}

