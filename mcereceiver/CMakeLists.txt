# ----------------------------------------------------------------------------
# Title      : ROGUE RSSI Receiver In C++
# ----------------------------------------------------------------------------
# File       : CMakeLists.txt
# Created    : 2018-02-27
# ----------------------------------------------------------------------------
# This file is part of the rogue_example software. It is subject to 
# the license terms in the LICENSE.txt file found in the top-level directory 
# of this distribution and at: 
#    https://confluence.slac.stanford.edu/display/ppareg/LICENSE.html. 
# No part of the rogue_example software, including this file, may be 
# copied, modified, propagated, or distributed except according to the terms 
# contained in the LICENSE.txt file.
# ----------------------------------------------------------------------------

# Check cmake version
cmake_minimum_required(VERSION 2.8)
include(InstallRequiredSystemLibraries)

# Project name
project (mcereceiver)

# C/C++
enable_language(CXX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

SET(BUILD_SHARED_LIBS OFF)

#find cppzmq wrapper, installed by make of cppzmq
find_package(cppzmq PATHS $ENV{CPPZMQ_PATH})

#####################################
# Boost Configuration
#####################################
set(Boost_USE_MULTITHREADED ON)

# Boost may need help on SLAC machines
set(BOOST_ROOT:PATHNAME $ENV{BOOST_PATH})

find_package(Boost 1.58 COMPONENTS system thread)

include_directories(../common/)
include_directories(${Boost_INCLUDE_DIRS})

# Set output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)


message("")
message("----------------------------------------------------------------------")
message("-- Success!")
message("")
message("-- Found cppzmq: ${cppzmq_INCLUDE_DIRS}")
message("")
message("-- Found boost: ${Boost_INCLUDE_DIRS}")
message("")

# Compile each source
file(GLOB APP_SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)
foreach (srcFile ${APP_SOURCES})
   get_filename_component(binName ${srcFile} NAME_WE)
   add_executable(${binName} ${srcFile})
   TARGET_LINK_LIBRARIES(${binName} cppzmq ${Boost_LIBRARIES})
endforeach ()

