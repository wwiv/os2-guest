
#include "copyPasteCompat.h"
#include "copypaste.h"
#include <stdint.h>
#include <stdio.h>

#define INCL_KBD
#define INCL_DOSPROCESS
#include <os2.h>


int main(int argc, char* argv[]) {
  fprintf(stdout, "OS2 Guest Helper for VMWare.\r\n");
  
  StartCopyPasteThread();
  KBDKEYINFO key{};

  for (;;) {
    DosSleep(1000);
    KbdPeek(&key, 0);
    if (key.fbStatus != 0) {
      StopCopyPasteThread();
      break;
    }

  }
  
  return 0;
}
