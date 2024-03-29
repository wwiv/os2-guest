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
#ifndef INCLUDED_HOST_H
#define INCLUDED_HOST_H

#include <stdint.h>
#include "vmtypes.h"

/**
 * Represents a point on the Host, in the format of the host operating
 * system.
 */
typedef struct {
  int16_t x;
  int16_t y;
} host_point ;

bool pointer_in_host(const host_point* pos);

/**
 * Module for interfacing with a VMWare Host through the
 * Host Backdoor API
 */

/** Gets the host pointer position */
bool get_host_pointer(host_point* pos);

/** Sets the host pointer position */
bool set_host_pointer(const host_point* pos);

/** Sets the host clipboard contents */
bool set_host_clipboard(char* b);

/** Gets the host clipboard contents or NULL if none exist */
char* get_host_clipboard();


#endif






