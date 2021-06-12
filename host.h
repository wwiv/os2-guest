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
/**
 * Represents a point on the Host, in the format of the host operating
 * system.
 */
struct host_point {
  int16_t x;
  int16_t y;
};

bool pointer_in_host(const host_point& pos);

/**
 * Class to encapsulate interfacing with a VMWare Host through the
 * Host Backdoor API
 */
class Host {
 public:
  Host();
  ~Host();
  
  /** Gets the host pointer position */
  host_point pointer();

  /** Sets the host pointer position */
  bool pointer(const host_point& pos);

  /** Sets the host clipboard contents */
  bool clipboard(char* b);

  /** Gets the host clipboard contents or NULL if none exist */
  char* clipboard();
};

#endif



