#
# NAME
#    dlmodules.tcl
#
# DESCRIPTION
#    Load various dlsh related modules
#
#

package provide dlsh 1.2

if { [lsearch [array names env] "DLSH_LIBRARY"] < 0 } {
   if { [file exists /usr/local/lib/tcltk/dlsh] } { 
      set env(DLSH_LIBRARY) /usr/local/lib/tcltk/dlsh
   } elseif { [file exists /usr/local/lib/dlsh] } { 
      set env(DLSH_LIBRARY) /usr/local/lib/dlsh
}
}

if { [array names env OS] == "OS" } {
    set ARCH [string tolower $env(OS)]
} else {
    set ARCH [string tolower [exec uname -s]]
}
if { ![regexp windows $ARCH] } {
    set DLSHMODDIR ${dlsh_library}/modules/$ARCH
} elseif { [info exists $env(DLSH_LIBRARY)] } {
    set DLSHMODDIR $env(DLSH_LIBRARY)/modules/$ARCH
} elseif { [file exists d:/usr/local/lib/dlsh] } {
    set DLSHMODDIR d:/usr/local/lib/dlsh/modules/$ARCH
} elseif { [file exists c:/usr/local/lib/dlsh] } {
    set DLSHMODDIR c:/usr/local/lib/dlsh/modules/$ARCH
}

set DLLEXT [info sharedlibextension]

proc default_dlsh_modules {} {
    if { ![regexp windows $::ARCH] } {
	load_Dli
	load_Fit
	load_Stats
    }
}

proc load_Tix {} {
    load $::DLSHMODDIR/libtix${::DLLEXT} Tix
}

proc load_Expect {} {
    load /usr/local/lib/libexpect5.20${::DLLEXT} Expect
}

proc load_Gd {} {
    load /usr/local/lib/gdCmd${::DLLEXT} Gdtcl
}

# Tk graphics
proc load_Cgwin {} {
    global DLSHMODDIR
    if { $::ARCH == "windows_nt" } {
	load $DLSHMODDIR/cg_win32${::DLLEXT} 
    } else {
	if { [info commands raster] == "" } {
	    load $DLSHMODDIR/cgwin${::DLLEXT}
	}
	namespace inscope :: {
	    proc cgwin { path args } { 
		eval raster $path $args
		$path initgraphics 
		return $path
	    }
	}
    }
}

proc load_Tkogl {} {
    global DLSHMODDIR ARCH
    if { $ARCH != "irix" } { 
	load $DLSHMODDIR/tkogl${::DLLEXT} Tkogl 
	load $DLSHMODDIR/dlgl${::DLLEXT} Dlgl 
    }
}

# Curve fitting module
proc load_Dlf {} {
    global DLSHMODDIR
    load $DLSHMODDIR/tcl_dlf${::DLLEXT} Dlf
}

# Image module
proc load_Dli {} {
    global DLSHMODDIR
    load $DLSHMODDIR/tcl_dli${::DLLEXT} Dli
}

# Stats module
proc load_Stats {} {
    global DLSHMODDIR
    load $DLSHMODDIR/tcl_stats${::DLLEXT} Stats
}

# Fitls module
proc load_Fit {} {
    global DLSHMODDIR
    load $DLSHMODDIR/tcl_fitls${::DLLEXT} Fit
}

# Impro module
proc load_Impro {} {
    global DLSHMODDIR
    load $DLSHMODDIR/tcl_impro${::DLLEXT} Impro
}

# Imsl module
proc load_Imsl {} {
    global DLSHMODDIR
    load $DLSHMODDIR/tcl_imslmath${::DLLEXT} Imsl
}

# Imsl stats module
proc load_Imsls {} {
    global DLSHMODDIR
    load $DLSHMODDIR/tcl_imslstat${::DLLEXT} Imsls
}

# Analog data file module
proc load_Adf {} {
    global DLSHMODDIR
    load $DLSHMODDIR/adf${::DLLEXT} Adf
}

# Canvas items using dlsh commands
proc load_Dlcanvas {} {
    global DLSHMODDIR
    load $DLSHMODDIR/dlcanvas${::DLLEXT}
}

# Mathwizards' MXT Module
proc load_Mxt {} {
    global DLSHMODDIR env dlsh_library
    set env(MVPATH) ${dlsh_library}/mxt/m-files
    load $DLSHMODDIR/mxt${::DLLEXT}
}

# Mathwizards' MXTGR Module
proc load_Mxtgr {} {
    global DLSHMODDIR
    load $DLSHMODDIR/mxtgr${::DLLEXT}
    namespace inscope :: { 
	proc mxt_demo {} { 
	    global dlsh_library
	    source ${dlsh_library}/mxt/demo.tcl
	}
    }
}

# Freetype functions
proc load_Freetype {} {
    global DLSHMODDIR
    load $DLSHMODDIR/tcl_freetype${::DLLEXT}
}

# Freetype functions
proc load_Wav {} {
    global DLSHMODDIR
    load $DLSHMODDIR/tcl_wav${::DLLEXT}
}

# Intel's image processing library
proc load_Ipl {} {
    global DLSHMODDIR ARCH
    if { [regexp windows $ARCH] } {
	load $DLSHMODDIR/tcl_ipl${::DLLEXT}
    }
}

# Intel's signal processing library
proc load_Nsp {} {
    global DLSHMODDIR ARCH
    if { [regexp windows $ARCH] } {
	load $DLSHMODDIR/tcl_nsp${::DLLEXT}
    }
}

# Matlab interface module
proc load_Matlab {} {
    global DLSHMODDIR
    load $DLSHMODDIR/mateng${::DLLEXT}
}

