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
#include "vmtypes.h"

/**
 * Represents a point on the Host, in the format of the host operating
 * system.
 */
typedef struct {
  int16_t x;
  int16_t y;
} guest_point ;

typedef struct {

  unsigned long hab_;
  unsigned long hmq_;
  unsigned long screen_max_y_;
  guest_point last_point_;
} guest_info ;


/**
 * Code to encapsulate interfacing with a VMWare Guest.
 */

bool guest_init(guest_info* guest);
bool guest_destroy(guest_info* guest); 

/** Gets the guest pointer position */
bool get_guest_pointer(guest_info*, guest_point*);

/** Sets the guest pointer position */
bool set_guest_pointer(guest_info*, const guest_point* pos);

/** Shows or hides the pointer  */
bool set_guest_pointer_visible(guest_info*,bool visible);

/** Sets the host clipboard contents */
bool set_guest_clipboard(guest_info*, char* b);

/** Gets the host clipboard contents or NULL if none exist */
char* get_guest_clipboard(guest_info*);

/** converts a host to guest point */
void host_to_guest(guest_info*, const host_point* hp, guest_point*);

/** converts a host to guest point */
void guest_to_host(guest_info*, const guest_point* gp, host_point*);



#endif
