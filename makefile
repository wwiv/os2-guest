#########################################################
# Makefile: OS/2 Guest Tools for VMWare
#

CC = wcl386
AS = wasm

# debug info and warning level 3
CFLAGS	= -d1 -w3
ASFLAGS = -d1
LDFLAGS = debug all
SYSTEM	= os2v2

# Binary and Objects
EXE	= vmtools.exe
OBJS 	= backdoor.obj 	&
	log.obj 	&
	guest.obj 	&
	host.obj 	&
	vmtools.obj

#########################################################
# Makefile rules 

.cpp.obj:
	$(CC) $(CFLAGS) -c $<

.asm.obj:
	$(AS) $(ASFLAGS) $<

$(EXE): $(OBJS)
	wlink system $(SYSTEM) $(LDFLAGS) name $(EXE) &
        file {$(OBJS)}

clean: .SYMBOLIC
	-del *.obj
	-del $(EXE)





