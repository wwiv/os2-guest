# os2-guest
# OS2 guest support for VMWare

Simple utilities for VMWare guest support for OS/2

This includes LGPL code from https://github.com/vmware/open-vm-tools
for the backdoor API for VMWare.

# Prerequisites

os2-guest needs watcom 1.9 for OS/2


# Get the source

- Clone the repository then build with wmake.

```
git clone https://github.com/wwiv/os2-guest
```

# Building

```
wmake
```

# Running
Just run vmtoolsd.exe

If you would like debug logs, add -D# and -Llogfilename to the commandline of vmtoolsd.exe
where # is a number from 1-10, the higher number the more log data that will be written
to the file specified by the -L parameter.

# Stopping
Use vmtoolsctl to stop. Example:
```
vmtoolsctl -q
```





