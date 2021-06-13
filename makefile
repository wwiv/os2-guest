#########################################################
# Makefile: OS/2 Guest Tools for VMWare
#

CC = wcl386
AS = wasm

# debug info and warning level 3
CFLAGS		= -d1 -w3
ASFLAGS 	= -d1
LDFLAGS 	= debug all
SYSTEM		= os2v2
SYSTEM_PM	= os2v2_pm

# Binary and Objects for vmtoolsd
VMTOOLSD_EXE	= vmtoolsd.exe
VMTOOLSD_OBJS 	= backdoor.obj 	&
		log.obj 	&
		guest.obj 	&
		host.obj 	&
		vmtoolsd.obj

# Binary and Objects for vmtoolsd
VMTOOLSCTL_EXE	= vmtoolsctl.exe
VMTOOLSCTL_OBJS = log.obj 	&
		vmtoolsctl.obj

#########################################################
# Makefile rules 

.cpp.obj:
	$(CC) $(CFLAGS) -c $<

.asm.obj:
	$(AS) $(ASFLAGS) $<

all:	$(VMTOOLSD_EXE) $(VMTOOLSCTL_EXE)

$(VMTOOLSD_EXE): $(VMTOOLSD_OBJS)
	wlink system $(SYSTEM_PM) $(LDFLAGS) name $(VMTOOLSD_EXE) &
        file {$(VMTOOLSD_OBJS)}

$(VMTOOLSCTL_EXE): $(VMTOOLSCTL_OBJS)
	wlink system $(SYSTEM) $(LDFLAGS) name $(VMTOOLSCTL_EXE) &
        file {$(VMTOOLSCTL_OBJS)}

clean: .SYMBOLIC
	-del *.obj
	-del $(VMTOOLSCTL_EXE)
	-del $(VMTOOLSD_EXE)





