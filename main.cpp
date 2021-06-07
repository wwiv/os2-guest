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
#include "copypaste.h"
#include <stdint.h>
#include <stdio.h>

#define INCL_KBD
#define INCL_DOSPROCESS
#include <os2.h>


int main(int argc, char* argv[]) {
  fprintf(stdout, R"(
OS2 Guest Helper for VMWare.
See https://github.com/wwiv/os2-guest for more information.

This program uses portions of [https://github.com/vmware/open-vm-tools]
which are licensed under the GNU Lesser General Public Library, version 2.1


Press the ESCAPE key to exit.

)");
  
  StartCopyPasteThread();
  KBDKEYINFO key{};

  for (;;) {
    DosSleep(1000);
    KbdPeek(&key, 0);
    if (key.fbStatus != 0 && key.chChar == 27 /* ESC */) {
      StopCopyPasteThread();
      break;
    }
  }
  
  return 0;
}






