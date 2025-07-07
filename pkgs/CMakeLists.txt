cmake_minimum_required(VERSION 3.15)

#
# Add our own tcl-only packages to dlsh.zip.
#

install(DIRECTORY ${CMAKE_SOURCE_DIR}/lib/local/corrgram TYPE LIB COMPONENT dlsh-zip)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/lib/local/graphing TYPE LIB COMPONENT dlsh-zip)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/pkgs/hex TYPE LIB COMPONENT dlsh-zip)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/lib/local/mtspec TYPE LIB COMPONENT dlsh-zip)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/lib/local/phase TYPE LIB COMPONENT dlsh-zip)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/lib/local/rasts TYPE LIB COMPONENT dlsh-zip)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/pkgs/qpcs TYPE LIB COMPONENT dlsh-zip)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/lib/local/robust TYPE LIB COMPONENT dlsh-zip)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/lib/local/roc TYPE LIB COMPONENT dlsh-zip)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/lib/local/selectivity TYPE LIB COMPONENT dlsh-zip)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/lib/local/spec TYPE LIB COMPONENT dlsh-zip)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/lib/local/spikes TYPE LIB COMPONENT dlsh-zip)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/lib/local/stimcompose TYPE LIB COMPONENT dlsh-zip)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/lib/local/xCorr TYPE LIB COMPONENT dlsh-zip)

#
# Add our non-public dependencies to dlsh.zip
#

if(DEFINED DLSH_DEPENDENCIES)
    install(DIRECTORY ${DLSH_DEPENDENCIES}/mclistbox-1.03 TYPE LIB COMPONENT dlsh-zip)
endif()

#
# Add external tcl-only packages to dlsh.zip.
#

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

#
# Add external tcl-plus-c packages to dlsh.zip.
#

set(SQLITE_VERSION "version-3.50.2")
install(
    CODE "execute_process(WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/deps COMMAND git clone --depth 1 --branch ${SQLITE_VERSION} https://github.com/sqlite/sqlite.git)"
    CODE "execute_process(WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/deps/sqlite COMMAND ./configure --enable-all)"
    CODE "set(ENV{TCLLIBPATH} ${CMAKE_INSTALL_PREFIX}/lib)"
    CODE "execute_process(WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/deps/sqlite COMMAND make tclextension-install)"
    COMPONENT dlsh-zip
)
set(YAJLTCL_VERSION "v1.8.1")
install(
    CODE "execute_process(WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/deps COMMAND git clone --depth 1 --branch ${YAJLTCL_VERSION} https://github.com/flightaware/yajl-tcl.git)"
    CODE "execute_process(WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/deps/yajl-tcl COMMAND autoconf)"
    CODE "execute_process(WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/deps/yajl-tcl COMMAND ./configure --exec_prefix=${CMAKE_INSTALL_PREFIX})"
    CODE "execute_process(WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/deps/yajl-tcl COMMAND make)"
    CODE "execute_process(WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/deps/yajl-tcl COMMAND make install)"
    COMPONENT dlsh-zip
)

#
# Add our own tcl-plus-c packages to dlsh.zip
#

# Our packages use pkgIndex.tcl that expect shared libs in platform- and machine-specific subdirs.
# Define these once here to use in package builds included below.
if(WIN32)
    set(TCL_OS "Windows NT")
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "ARM")
        set(TCL_MACHINE "arm64")
    else()
        set(TCL_MACHINE "amd")
    endif()
elseif(APPLE)
    set(TCL_OS "Darwin")
    set(TCL_MACHINE "arm64")
else()
    set(TCL_OS "Linux")
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch" OR CMAKE_SYSTEM_PROCESSOR MATCHES "arm")
        set(TCL_MACHINE "aarch64")
    else()
        set(TCL_MACHINE "x86_64")
    endif()
endif()

add_subdirectory(${CMAKE_SOURCE_DIR}/pkgs/bink)

include(CPack)
cpack_add_component(dlsh-zip)
