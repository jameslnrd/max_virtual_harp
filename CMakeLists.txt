# Copyright 2018 The Min-DevKit Authors. All rights reserved.
# Use of this source code is governed by the MIT License found in the License.md file.

cmake_minimum_required(VERSION 3.0)

set(C74_MIN_API_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../min-api)
include(${C74_MIN_API_DIR}/script/min-pretarget.cmake)


set(GCC_COVERAGE_COMPILE_FLAGS "-mavx2 -mfma")
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")

#set(GCC_COVERAGE_LINK_FLAGS    "-lgcov")
#set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")

#############################################################
# MAX EXTERNAL
#############################################################


include_directories( 
	"${C74_INCLUDES}"
)


set( SOURCE_FILES
	${PROJECT_NAME}.cpp
	BowInput.cpp
	Bridge.cpp
	Dampers.cpp
	Globals.cpp
	Obstacle.cpp
	StringPhysics.cpp

)


set(HEADERS
   ${HEADERS}
	${PROJECT_NAME}.h
	BowInput.h
	Bridge.h
	Dampers.h
	Globals.h
	Obstacle.h
	StringPhysics.h
)


add_library( 
	${PROJECT_NAME} 
	MODULE
	${SOURCE_FILES}
)


include(${C74_MIN_API_DIR}/script/min-posttarget.cmake)


#############################################################
# UNIT TEST
#############################################################

#include(${C74_MIN_API_DIR}/test/min-object-unittest.cmake)
