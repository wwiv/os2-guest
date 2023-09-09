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
#include "log.h"

// Use stdint.h vs. cstdint so types are in global namespace, not std::
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define INCL_PM
#define INCL_DOS
#define INCL_ERRORS
#include <os2.h>

// Logs at a higheer level than threshold will not be logged.
static int LOG_THRESHOLD = 0;
static FILE* logh = stderr;

void set_loglevel(int lvl) {
  LOG_THRESHOLD = lvl;
}

void set_logfile(const char* fn) {
  logh = fopen(fn, "a");
  if (!logh) {
    logh = stderr;
  }
}

void log(const char* s) {
  fprintf(logh, "%s\r\n", s);
  fflush(logh);
}

/**
 * Log at log level lvl. 
 * higher levels mean more logging.  Use 1-4.
 */
void logl(int lvl, const char* s) {
  if (lvl <= LOG_THRESHOLD) {
    log(s);
  }
}

void loglf(int lvl, const char* msg, ...) {
  char buf[1024];
  va_list argptr;
  va_start(argptr, msg);
  vsprintf(buf, msg, argptr);
  va_end(argptr);
  logl(lvl, buf);
}









