############################################################################### 
#
# CMake for OVT
# Part of OVT used by OS2 Guest
#

add_library(ovt_backdoor
	    backdoor/backdoor.c
	    backdoor/backdoorGcc32.c
	    hostinfoHV.c
)
target_include_directories(ovt_backdoor PUBLIC include)

add_library(ovt_copypaste
	    cp/copyPasteCompat.cpp
	    pointer.cpp
)
target_include_directories(ovt_copypaste PUBLIC include)

