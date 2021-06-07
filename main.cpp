
#include "copyPasteCompat.h"
#include "copypaste.h"
#include <stdint.h>
#include <stdio.h>

#define INCL_DOSPROCESS
#include <os2.h>


int main(int argc, char* argv[]) {
  fprintf(stdout, "OS2 Guest Helper for VMWare.\r\n");
  
  int32_t hclen = CopyPaste_GetHostSelectionLen();
  fprintf(stdout, "Host copy len: %d\r\n", hclen);

  if (hclen < 1024) {
    char buf[1028];
    if (hclen > 1024) {
      hclen = 1024;
    }
    CopyPaste_GetHostSelection(hclen, buf);
    fprintf(stdout, "Clipboard contents: \r\n%s\r\n", buf);
  }

  StartCopyPasteThread();
  for (;;) {
    DosSleep(1000);
  }
  
  return 0;
}
