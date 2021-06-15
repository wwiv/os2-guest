OS/2 Guest support for VMWare

OS/2 Guest support for VMWare provides the following guest services for OS/2
guests running under VMWare:

* Clipboard Syncronization - the contents of the clipboard are shared between
  the guest and the host.

* Mouse pointer synchronization - The mouse pointer can leave the guest VM
  without needing to ungrab it (Alt-Control).  Also when the mouse pointer
  is grabbed by the guest, usually when you click inside the VM, the mouse
  pointer should be in the right location for the guest and not the previous
  location.

Installation
============

1) Put vmtoolsd.exe and vmtoolsctl.exe in some directory in your path,
   such as C:\usr\local\bin or C:\bin or the like.

2) Add a line in C:\startup.cmd to launch vmtoolsd.exe using "start /PM".
   You don't need to specify any paramater.  Example:
   
   start /PM C:\BIN\VMTOOLSD.EXE

3) If you don't want to automatically start it when os/2 boots up, you
   may run it manually using the same command as in startup.cmd. 


Use
===

Once installed that's it.  You can stop it by running "vmtoolsctl -q", or by
killing the process manually from top.

To enable debugging you can use the following parameters to vmtoolsd.exe

-D# (where # is 0-4, higher value means more debugging)
-LXXX (where XXX is the name of the log file)

Example:

	START /PM vmtoolsd.exe -D3 -LC:\logs\vmtoolsd.log



Known Limitations
=================
* The mouse pointer synchronization doesn't work when run from a RUN= line
  in config.sys, or startup.cmd without using "START /PM" or detaching 
  in the current command prompt, only when run from a command session prompt.

See http://github.com/wwiv/os2-guest/issues for the full list.


Soure Code and Help
===================
OS/2 Guest support for VMWare is Free and Open Sourec Software (FOSS), the
source is available on http://github.com/wwiv/os2-guest.

There is no guaranteed support, and source is available.

Enjoy
