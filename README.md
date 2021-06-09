# os2-guest
# OS2 guest support for VMWare

Simple utilities for VMWare guest support for OS/2

This includes LGPL code from https://github.com/vmware/open-vm-tools
for the backdoor API for VMWare.

# Prerequisites

os2-guest needs cmake and gcc 9.2.


# Get the source

- Clone the repository then build with cmake.

```
git clone https://github.com/wwiv/os2-guest
```

# Building

```
mkdir _b
cd _b
cmake ..
cmake --build .
```

Next time you build you only need the last line:
```
cmake --build .
```


# Running
Just run os2-guest.exe

There's no command line arguments, just run it and ignore it.  To exit it, type the ESCAPE key
in the console running it.




