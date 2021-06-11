;;   OS/2 Guest Tools for VMWare
;;   Copyright (C)2021, Rushfan
;;
;;  Licensed  under the  Apache License, Version  2.0 (the "License");
;;; you may not use this  file  except in compliance with the License.
;;   You may obtain a copy of the License at                          
;; 
;;     http://www.apache.org/licenses/LICENSE-2.0
;;
;;   Unless  required  by  applicable  law  or agreed to  in  writing,
;;   software  distributed  under  the  License  is  distributed on an
;;   "AS IS"  BASIS, WITHOUT  WARRANTIES  OR  CONDITIONS OF ANY  KIND,
;;   either  express  or implied.  See  the  License for  the specific
;;   language governing permissions and limitations under the License.

	TITLE BackDoor API for VMWare
	PAGE 55,132
	.386
	
DGROUP  group   _DATA

_DATA   segment word public 'DATA'
		
;; MAGIC number to send to backdoor api
BDOOR_MAGIC	equ	564D5868H
	
;; Low-bandwidth backdoor port number 
;; for the IN/OUT interface.
BDOOR_PORT	equ	5658H

_DATA   ends

assume  cs:_TEXT,ds:DGROUP

_TEXT   segment word public 'CODE'
        assume  CS:_TEXT

	;; int Backdoor(int)
	public Backdoor_
Backdoor_	proc	near
 		mov ecx, eax
		mov eax, BDOOR_MAGIC
		mov ebx, 0
		mov dx, BDOOR_PORT
		in eax, dx
		ret
Backdoor_	endp
	
	;; int Backdoor2(int, int)
	public Backdoor2_
Backdoor2_	proc	near
 		mov ecx, eax
		mov ebx, edx
		;; 		xor edx, edx
		mov eax, BDOOR_MAGIC
		mov dx, BDOOR_PORT
		in eax, dx
		ret
Backdoor2_	endp
	
_TEXT   ends
        end
