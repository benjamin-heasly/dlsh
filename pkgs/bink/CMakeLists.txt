cmake_minimum_required(VERSION 3.15)

project(tcl_bink)

set(CMAKE_BUILD_TYPE Release)

add_definitions(-fPIC -DUSE_INTERP_RESULT -DUSE_TCL_STUBS)

if(WIN32)
	include_directories(c:/usr/local/include)
	link_directories(c:/usr/local/lib)
elseif(APPLE)
	add_definitions(-DMACOS -Dunix -DLINUX)
	include_directories(/usr/local/include)
	link_directories(/usr/local/lib)
	set(BUNDLE_LOAD "-dynamiclib")
	set(TCLLIB tclstub8.6)
	set(BINK /usr/local/lib/libBink2MacArm64.dylib)
elseif(CMAKE_SYSTEM_NAME STREQUAL "QNX")
	include_directories(../local/include)
	link_directories(../local/aarch64/lib)
	set(TCLLIB tclstub8.6)
else()
	add_definitions(-DLINUX)
	include_directories(${APP_DIR} /usr/local/include)
	link_directories(/usr/local/lib)
	set(TCL_INCLUDE_DIR /usr/include/tcl8.6)
	set(TCLLIB tclstub8.6)
	set(BINK /usr/local/lib/bink2lnx64.a)
endif()

include_directories(${TCL_INCLUDE_DIR} .)

add_library(tcl_bink SHARED tcl_bink.c lodepng.c)


target_link_libraries(tcl_bink ${TCLLIB} ${BINK})

if (APPLE)
 add_custom_command(TARGET tcl_bink
    POST_BUILD COMMAND 
    ${CMAKE_INSTALL_NAME_TOOL} -change "@executable_path/libBink2MacArm64.dylib" "@rpath/libBink2MacArm64.dylib" $<TARGET_FILE:tcl_bink>)
endif()


