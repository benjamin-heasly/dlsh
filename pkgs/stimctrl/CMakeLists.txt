cmake_minimum_required(VERSION 3.15)
set(CMAKE_CXX_STANDARD 11)
project(stimctrl)

set(CMAKE_BUILD_TYPE Release)

add_definitions(-DUSE_TCL_STUBS)

if(WIN32)
	include_directories(c:/usr/local/include)
	link_directories(c:/usr/local/lib)
elseif(APPLE)
	add_definitions(-DMACOS -Dunix -DLINUX -fPIC)
	include_directories(/usr/local/include)
	link_directories(/usr/local/lib)
	set(LABLIB lab)
elseif(CMAKE_SYSTEM_NAME STREQUAL "QNX")
	add_definitions(-fPIC)
	include_directories(../local/include)
	link_directories(../local/aarch64/lib)
	set(TCLLIB tclstub)
else()
	add_definitions(-DLINUX -fPIC)
	include_directories(${APP_DIR} /usr/local/include)
	link_directories(/usr/local/lib)
endif()

include_directories(${TCL_INCLUDE_DIR} .)

###############################
# stimctrl
###############################
if (WIN32)
add_library(stimctrl SHARED stimctrl_w32.c)
else()
add_library(stimctrl SHARED tclstimctrl.c sockapi.c)
endif()

if(WIN32)
	set(TCLLIB c:/usr/local/lib/tclstub.lib)
	set(DEF_FILE ${CMAKE_CURRENT_SOURCE_DIR}/stimctrl.def)
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /INCREMENTAL /LTCG /NODEFAULTLIB:libcmt /NODEFAULTLIB:MSCVRT /OUT:libstimctrl.dll")	
elseif(APPLE)
	set(TCLLIB tclstub)
	set(BUNDLE_LOAD "-dynamiclib")
else()
	set(TCLLIB tclstub)
endif()

target_link_libraries(stimctrl ${TCLLIB})

