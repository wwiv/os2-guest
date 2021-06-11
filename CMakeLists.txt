############################################################################### 
#
# CMake for WWIV

cmake_minimum_required(VERSION 3.13 FATAL_ERROR)
project(os2-guest)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

# add_compile_options(-S)

if (NOT OS2)
   message(FATAL_ERROR "This requires OS/2")
endif()

if (NOT CMAKE_COMPILER_IS_GNUCC)
   message(FATAL_ERROR "Requires GCC")
endif()

if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.2)
   message(FATAL_ERROR "Require at least gcc-9.2; found: ${CMAKE_CXX_COMPILER_VERSION}")
endif() 

add_subdirectory(ovt)

add_executable(os2-guest main.cpp copypaste.cpp)

target_include_directories(os2-guest PRIVATE ovt/cp)
target_link_libraries(os2-guest ovt_copypaste ovt_backdoor)


