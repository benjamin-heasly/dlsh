cmake_minimum_required(VERSION 3.15)

add_subdirectory( libdg )

if(NOT DEFINED PROJECT_VERSION)
  set(PROJECT_VERSION 0.0.0)
endif()
project(dlsh VERSION ${PROJECT_VERSION})

set(CMAKE_BUILD_TYPE Release)
if (WIN32)
    add_definitions(-DUSE_TCL_STUBS)
else()
    add_definitions(-fPIC -DUSE_TCL_STUBS)
endif()

if(WIN32)
    include_directories(src src/lablib c:/tcl/include c:/usr/local/include c:/usr/local/include/hpdf)
    link_directories(c:/usr/local/lib/$ENV{VSCMD_ARG_TGT_ARCH})
elseif(APPLE)
    add_definitions(-DMACOS -Dunix -DLINUX)
    include_directories(src src/lablib /usr/local/include /opt/homebrew/include/ /opt/homebrew/include/tcl-tk)
    link_directories(/usr/local/lib /opt/homebrew/lib)
elseif(CMAKE_SYSTEM_NAME STREQUAL "QNX")
    add_definitions(-DLINUX)
    include_directories(../local/include)
    link_directories(../local/aarch64/lib)
    set(TCLLIB tclstub)
    set(PDFLIB hpdf)
    set(LZ4LIB lz4)
    set(ZLIB z)
else()
    add_definitions(-DLINUX)
    include_directories(src src/lablib ${APP_DIR} /usr/local/include)
    link_directories(/usr/local/lib)
    set(LIBDL dl)
endif()

include_directories(${TCL_INCLUDE_DIR} src)

###############################
# dlsh
###############################
add_library(dlsh SHARED src/dfana.c src/tcl_dlg.c src/dfevt.c src/dlsh_pkg.c src/tcl_df.c src/tcl_dm.c src/dlarith.c src/dmana.c src/tcl_dl.c src/dgjson.c src/lablib/gbufutl.c src/lablib/gbuf.c src/lablib/cg_ps.c src/lablib/cg_base.c src/lablib/axes.c src/lablib/cgraph.c src/lablib/timer.c src/lablib/utilc_unix.c src/lablib/randvars.c src/lablib/prmutil.c src/lablib/dfutils.c src/lablib/df.c src/lablib/dynio.c src/lablib/rawapi.c src/lablib/lodepng.c src/lablib/lz4utils.c src/lablib/dslog.c )
set_target_properties(dlsh PROPERTIES PUBLIC_HEADER "src/dfana.h;src/tcl_dl.h;src/lablib/cgraph.h;src/lablib/gbuf.h;src/lablib/utilc.h;")

if(WIN32)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(DEF_FILE ${CMAKE_CURRENT_SOURCE_DIR}/src/dlsh.def)
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /INCREMENTAL /LTCG /NODEFAULTLIB:libcmt /NODEFAULTLIB:MSCVRT /OUT:libdlsh.dll /DEF:${DEF_FILE} /IMPLIB:libdlsh.lib")
        set(TCLLIB tclstub.lib)
        set(ZLIB zlibstatic.lib)
        set(LZ4LIB liblz4_static.lib)
        set(PDFLIB hpdf.lib)
        set(LIBJANSSON jansson.lib)
    endif()
elseif(APPLE)
    set(TCLLIB tclstub)
    set(ZLIB z)
    find_library(PDFLIB NAMES "libhpdf.a")
    find_library(LIBJANSSON NAMES "libjansson.a")
    find_library(LZ4LIB NAMES "liblz4.a")
    set(BUNDLE_LOAD "-dynamiclib")
    # set(CMAKE_EXE_MODULE_FLAGS "-arch arm64 -arch x86_64")
elseif(CMAKE_SYSTEM_NAME STREQUAL "QNX")
    set(LIBJANSSON jansson)
    set(LABLIB lab)
    set(TCLLIB tclstub)
    set(PDFLIB libhpdf.a)
    set(LZ4LIB lz4)
    set(ZLIB z)
else()
    find_library(LIBJANSSON NAMES "libjansson.a")
	set(LABLIB lab)
    set(TCLLIB tclstub)
    find_library(PDFLIB NAMES "libhpdf.a")
	set(LZ4LIB lz4)
    set(ZLIB z)
    set(LZ4LIB lz4)
    set(PDFLIB hpdf)
endif()
target_link_libraries( dlsh ${LIBJANSSON} ${TCLLIB} ${PDFLIB} ${LZ4LIB} ${ZLIB} )

set(CPACK_PACKAGE_NAME dlsh)
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Dynamic Lists and other TCL utilities.")
set(CPACK_PACKAGE_CONTACT SheinbergLab)

set(CPACK_PACKAGING_INSTALL_PREFIX "/usr/local")
install(TARGETS dlsh PUBLIC_HEADER COMPONENT dlsh)
install(TARGETS dlsh LIBRARY COMPONENT dlsh)

# Add our own tcl-only packages to dlsh.zip.
install(DIRECTORY ${CMAKE_SOURCE_DIR}/lib/local/corrgram TYPE LIB COMPONENT dlsh-zip)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/lib/local/graphing TYPE LIB COMPONENT dlsh-zip)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/pkgs/hex TYPE LIB COMPONENT dlsh-zip)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/lib/local/mtspec TYPE LIB COMPONENT dlsh-zip)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/lib/local/phase TYPE LIB COMPONENT dlsh-zip)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/lib/local/rasts TYPE LIB COMPONENT dlsh-zip)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/lib/local/robust TYPE LIB COMPONENT dlsh-zip)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/lib/local/roc TYPE LIB COMPONENT dlsh-zip)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/lib/local/selectivity TYPE LIB COMPONENT dlsh-zip)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/lib/local/spec TYPE LIB COMPONENT dlsh-zip)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/lib/local/spikes TYPE LIB COMPONENT dlsh-zip)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/lib/local/stimcompose TYPE LIB COMPONENT dlsh-zip)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/lib/local/xCorr TYPE LIB COMPONENT dlsh-zip)

if(DEFINED DLSH_DEPENDENCIES)
    # Add our non-public dependencies to dlsh.zip
    install(DIRECTORY ${DLSH_DEPENDENCIES}/mclistbox-1.03 TYPE LIB COMPONENT dlsh-zip)
endif()

# Add external tcl-only packages to dlsh.zip.
# tcllib and tklib use a standard configure/make pattern.
set(TCLLIB_VERSION "tcllib-2-0")
install(
    CODE "execute_process(WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/deps COMMAND git clone --depth 1 --branch ${TCLLIB_VERSION} https://github.com/tcltk/tcllib.git)"
    CODE "execute_process(WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/deps/tcllib COMMAND ./configure --prefix=${CMAKE_INSTALL_PREFIX})"
    CODE "execute_process(WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/deps/tcllib COMMAND make install-libraries)"
    COMPONENT dlsh-zip
)
set(TKLIB_VERSION "tklib-0.9")
install(
    CODE "execute_process(WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/deps COMMAND git clone --depth 1 --branch ${TKLIB_VERSION} https://github.com/tcltk/tklib.git)"
    CODE "execute_process(WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/deps/tklib COMMAND ./configure --prefix=${CMAKE_INSTALL_PREFIX})"
    CODE "execute_process(WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/deps/tklib COMMAND make install-libraries)"
    COMPONENT dlsh-zip
)
# TkCon is pure tcl, but the official pkgIndex.tcl comes from a template as pkgIndex.tcl.in.
# We can run configure/make to produce the pkgIndex.tcl we need.
# Annoyingly, configure fails without tk-dev installed.
# And we still have to manually copy the files we need!
set(TKCON_VERSION "tkcon-2-8")
install(
    CODE "execute_process(WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/deps COMMAND git clone --depth 1 --branch ${TKCON_VERSION} https://github.com/bohagan1/TkCon.git)"
    CODE "execute_process(WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/deps/TkCon COMMAND ./configure && make)"
    CODE "execute_process(WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}/lib COMMAND mkdir -p ${TKCON_VERSION})"
    CODE "execute_process(WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}/lib/${TKCON_VERSION} COMMAND cp ${CMAKE_SOURCE_DIR}/deps/TkCon/pkgIndex.tcl .)"
    CODE "execute_process(WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}/lib/${TKCON_VERSION} COMMAND cp ${CMAKE_SOURCE_DIR}/deps/TkCon/tkcon.tcl .)"
    COMPONENT dlsh-zip
)
# bwidget is just tcl files that we copy -- and we don't need the .git history.
set(BWIDGET_VERSION "bwidget-1-10-1")
install(
    CODE "execute_process(WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}/lib COMMAND git clone --depth 1 --branch ${BWIDGET_VERSION} https://github.com/tcltk/bwidget ${BWIDGET_VERSION})"
    CODE "execute_process(WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}/lib/${BWIDGET_VERSION} COMMAND rm -rf .git/)"
    COMPONENT dlsh-zip
)

include(InstallRequiredSystemLibraries)

if(WIN32)
    # TODO
elseif(APPLE)
    if(DEFINED "CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM")
        # Configure for apple code signing, notarization, and gatekeeper/entitlements.
        set(CMAKE_XCODE_ATTRIBUTE_OTHER_CODE_SIGN_FLAGS "--strict --timestamp --options=runtime")
        set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_INJECT_BASE_ENTITLEMENTS "NO")
        set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "Developer ID Application")
        set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_STYLE "Manual")
        set(CMAKE_OSX_DEPLOYMENT_TARGET "14.0")
        set(CMAKE_SKIP_RPATH TRUE)

        # Set up .pkg metadata.
        list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/macos)
        set(CPACK_PRODUCTBUILD_IDENTIFIER "org.sheinberglab")
        set(CPACK_RESOURCE_FILE_WELCOME ${CMAKE_SOURCE_DIR}/macos/welcome.txt)
        set(CPACK_RESOURCE_FILE_README ${CMAKE_SOURCE_DIR}/macos/readme.txt)
        set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_SOURCE_DIR}/macos/license.txt)
    endif()
else()
    set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
    set(CPACK_DEBIAN_PACKAGE_DEPENDS dlsh-dg)
endif()

include(CPack)
cpack_add_component(dlsh)
cpack_add_component(dlsh-zip)
