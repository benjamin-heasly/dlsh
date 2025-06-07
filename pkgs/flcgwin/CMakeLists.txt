cmake_minimum_required(VERSION 3.15)

project(flcgwin)

set(CMAKE_BUILD_TYPE Release)

add_definitions(-DUSE_TCL_STUBS)

if(WIN32)
    add_compile_options("/DFL_DLL")
	include_directories(c:/usr/local/include ../../src ../../src/lablib)
	link_directories(c:/usr/local/lib/$ENV{VSCMD_ARG_TGT_ARCH})
else(APPLE)
	add_definitions(-fPIC)
	include_directories(../../src ../../src/lablib)
	link_directories(../../build /usr/local/lib)
endif()

include_directories(${TCL_INCLUDE_DIR} .)

###############################
# flcgwin
###############################
add_library(flcgwin SHARED flcgwin.cpp cg_tab_manager.cpp)

if(WIN32)
	set_target_properties(flcgwin PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
	set(DLSH libdlsh.lib)
	set(TCLLIB tclstub.lib)
	set(FLTKLIB fltk_dll.lib)
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /INCREMENTAL /LTCG /NODEFAULTLIB:libcmt /NODEFAULTLIB:MSCVRT /OUT:libflcgwin.dll")
elseif(APPLE)
	set(TCLLIB tclstub)
	set(BUNDLE_LOAD "-dynamiclib -undefined dynamic_lookup")	
	set(DLSH dlsh)
else()
    set(DLSH dlsh)
	set(TCLLIB tclstub)
	set(ZLIB z)
endif()

if(APPLE)
 target_link_libraries(flcgwin ${TCLLIB} ${BUNDLE_LOAD} ${DLSH})
elseif(WIN32)
target_link_libraries(flcgwin ${TCLLIB} ${DLSH} ${FLTKLIB})
else()
target_link_libraries(flcgwin ${TCLLIB} ${DLSH})
endif()


