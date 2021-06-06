############################################################################### 
#
# CMake for WWIV

cmake_minimum_required(VERSION 3.13 FATAL_ERROR)
project(os2-guest)

if (NOT OS2)
   message(FATAL_ERROR "This requires OS/2")
endif()

if (NOT CMAKE_COMPILER_IS_GNUCC)
   message(FATAL_ERROR "Requires GCC")
endif()

if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.2)
   message(FATAL_ERROR "Require at least gcc-9.2; found: ${CMAKE_CXX_COMPILER_VERSION}")
endif() 

add_library(ovt
	    ovt/cp/copyPasteCompat.c
	    ovt/backdoor/backdoor.c
	    ovt/backdoor/backdoorGcc32.c
	    ovt/hostinfoHV.c
)

target_include_directories(ovt PUBLIC ovt/include)

add_executable(os2-guest os2-guest.c)

target_include_directories(os2-guest PRIVATE ovt/cp)
target_link_libraries(os2-guest ovt)


