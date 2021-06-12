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
#ifndef INCLUDED_GUEST_H
#define INCLUDED_GUEST_H

#include "host.h"

/**
 * Represents a point on the Host, in the format of the host operating
 * system.
 */
struct guest_point {
  int16_t x;
  int16_t y;
};

/**
 * Class to encapsulate interfacing with a VMWare Guest.
 */
class Guest {
 public:
  Guest();
  ~Guest();
  
  /** Gets the guest pointer position */
  guest_point pointer();

  /** Sets the guest pointer position */
  bool pointer(const guest_point& pos);

  /** Shows or hides the pointer  */
  bool pointer_visible(bool visible);

  /** Sets the host clipboard contents */
  bool clipboard(char* b);

  /** Gets the host clipboard contents or NULL if none exist */
  char* clipboard();

  /** converts a host to guest point */
  guest_point host_to_guest(const host_point& hp);

  /** converts a host to guest point */
  host_point guest_to_host(const guest_point& gp);

 private:
  unsigned long hab_;
  unsigned long hmq_;
  unsigned long screen_max_y_;
};


#endif
