#########################################################
# Makefile: OS/2 Guest Tools for VMWare
#

VERSION = 1.0
CC 	= wcl386
AS 	= wasm

# debug info and warning level 3
CFLAGS		= -d3 -w3 -hw
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

.c.obj:
	$(CC) $(CFLAGS) -c $<

.asm.obj:
	$(AS) $(ASFLAGS) $<

all:	config.h $(VMTOOLSD_EXE) $(VMTOOLSCTL_EXE)

.BEFORE
config.h: config.h.in 
	sed s/@VMTOOLS_VERSION@/$(VERSION)/ $< > $@

$(VMTOOLSD_EXE): config.h $(VMTOOLSD_OBJS)
	wlink system $(SYSTEM_PM) $(LDFLAGS) name $(VMTOOLSD_EXE) &
        file {$(VMTOOLSD_OBJS)}

$(VMTOOLSCTL_EXE): config.h $(VMTOOLSCTL_OBJS)
	wlink system $(SYSTEM) $(LDFLAGS) name $(VMTOOLSCTL_EXE) &
        file {$(VMTOOLSCTL_OBJS)}

DIST_DIR	= .\os2-guest-$(VERSION)
DIST_ZIP	= os2-guest-$(VERSION).zip
DIST_README	= $(DIST_DIR)\readme.txt
README		= docs\readme.txt

$(DIST_DIR): 
	mkdir $(DIST_DIR)

$(DIST_README): $(README)
	sed s/@VMTOOLS_VERSION@/$(VERSION)/ $< > $@

dist: config.h $(VMTOOLSD_EXE) $(VMTOOLSCTL_EXE) $(DIST_DIR) $(DIST_README) .SYMBOLIC
	-del $(DIST_DIR)\*.exe
	-del $(DIST_DIR)\*.txt
	copy $(VMTOOLSCTL_EXE) $(VMTOOLSD_EXE) $(DIST_DIR)
	sed s/@VMTOOLS_VERSION@/$(VERSION)/ $(README) > $(DIST_README)
	zip -r $(DIST_ZIP) $(DIST_DIR)\*

clean: .SYMBOLIC
	-del config.h
	-del *.obj
	-del $(VMTOOLSCTL_EXE)
	-del $(VMTOOLSD_EXE)
	-rm -fr $(DIST_DIR)
	-del $(DIST_ZIP)



