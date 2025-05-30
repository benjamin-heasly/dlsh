#!/bin/sh
# the next line restarts using wish \
exec wish84 "$0" "$@"

package require Tk
package require dlsh

load_Cgwin

set counter 1

# keeping track of notebook cgwin in an array - CGRAPH
array set CGRAPH ""
set CGRAPH(current_cgwin) ""
set CGRAPH(current_page) ""
set CGRAPH(notebook) ""
set CGRAPH(background) black
set CGRAPH(width) 640
set CGRAPH(height) 480
set CGRAPH(counter) -1

# parse command line args
set state flag
foreach arg $argv {
    switch -- $state {
	flag {
	    switch -glob -- $arg {
		-bg { set state bg } 
		-width { set state width }
		-height { set state height }
		-cfg { set state cfg }
		default { error "unknown flags $arg" }
	    }
	}
	bg {
	    set CGRAPH(background) $arg
	    set state flag
	}
	width {
	    set CGRAPH(width) $arg
	    set state flag
	}
	height {
	    set CGRAPH(height) $arg
	    set state flag
	}
	cfg {
	    if [file exists $arg] {
		set ::configfile $arg
		source $arg
		set ::configured 1
	    }
	    set state flag
	}
    }
}

wm title . "analysis shell"
wm protocol . WM_DELETE_WINDOW { wm iconify . }
console show 
console title "Analysis Shell Console"

proc configureMainWindow { } {
    global colors
    frame .mbar -relief raised -bd 2
    pack .mbar -side top -fill x
    
    menubutton .mbar.file -text File -underline 0 -menu .mbar.file.menu
    menubutton .mbar.edit -text Edit -underline 0 -menu .mbar.edit.menu
    menubutton .mbar.view -text View -underline 0 -menu .mbar.view.menu
    menubutton .mbar.help -text Help -underline 0 -menu .mbar.help.menu
    pack .mbar.file .mbar.edit .mbar.view -side left
    
    pack .mbar.help -side right
    menu .mbar.file.menu
    .mbar.file.menu add command -label "Print" -underline 0 \
	-underline 0 -command { dump_to_printer }
    
    .mbar.file.menu add command -label "Quit" -underline 0 \
	-accelerator "Ctrl+q" -command { close_cgsh }
    
    menu .mbar.view.menu
    .mbar.view.menu add cascade -label "Background" -underline 0 \
	-menu .mbar.view.menu.bg
    menu .mbar.view.menu.bg
    .mbar.view.menu.bg add command -label "White" \
	-underline 0 -command { 
	    set w [lindex [$::CGRAPH(current_cgwin) configure -width] 4]
	    set h [lindex [$::CGRAPH(current_cgwin) configure -height] 4]
	    destroy $::CGRAPH(current_cgwin)
	    pack [cgwin $::CGRAPH(current_cgwin) -width $w -height $h -bg white] \
		-fill both -expand true
	    setcolor $::colors(black)
	} 
    .mbar.view.menu.bg add command -label "Black" \
	-underline 0 -command { 
	    set w [lindex [$::CGRAPH(current_cgwin) configure -width] 4]
	    set h [lindex [ $::CGRAPH(current_cgwin) configure -height] 4]
	    destroy $::CGRAPH(current_cgwin)
	    pack [cgwin $::CGRAPH(current_cgwin) -width $w -height $h -bg black] \
		-fill both -expand true
	} 

    .mbar.view.menu add cascade -label "Display" -underline 0 \
	-menu .mbar.view.menu.display
    menu .mbar.view.menu.display
    .mbar.view.menu.display add command -label "Landscape Mode" \
	-underline 0 -command { 
	    $::CGRAPH(current_cgwin) configure -width 792 -height 612 
	    dlp_landscape
	} 
    .mbar.view.menu.display add command -label "Portrait Mode" \
	-underline 0 -command { 
	    $::CGRAPH(current_cgwin) configure -height 792 -width 612 
	    dlp_portrait
	} 

    menu .mbar.edit.menu
    .mbar.edit.menu add command -label "Copy" -underline 0 \
	-command {$::CGRAPH(current_cgwin) copy}
    
    menu .mbar.help.menu
    .mbar.help.menu add command -label "About" -underline 0
    
    tk_menuBar .mbar .mbar.file .mbar.edit .mbar.view .mbar.help

    # button bar for notebook interactions (create, delete, delete_all)
    set bfrm [frame .buttonframe]
    pack $bfrm  -side top -fill both
    set new [button $bfrm.new -text "New Tab" -command newTab -width 10]
    set close [button $bfrm.close -text "Close" -command {closeTab $::CGRAPH(current_page)} -width 10]
    set closeall [button $bfrm.closeall -text "Close All" -command closeAllTabs -width 10]
    pack $closeall $close $new -side right -anchor e

    # setup a Notebook to hold cgwins
    package require BWidget
    set winframe [frame .winframe]
    pack $winframe -fill both  -expand true
    set ::CGRAPH(notebook) [NoteBook $winframe.nb -ibd 0 -side top]
    # initialize first tab
    newTab "Main"
    $::CGRAPH(notebook) compute_size
    pack $::CGRAPH(notebook) -fill both -expand yes -padx 0 -pady 0
    $::CGRAPH(notebook) raise [$::CGRAPH(notebook) page 0]

    # some basic bindings    
    bind . <Control-q> { close_cgsh } 
    bind . <Control-c> { $::CGRAPH(current_cgwin) copy }
    bind . <Control-h> { toggle_console }
    
    # bindings to cycle through notebook with PageUp and PageDown (currently no wrapping, but could add it easily)
    bind . <Prior> {
	$CGRAPH(notebook) raise [lindex [$CGRAPH(notebook) pages] [expr [lsearch [$CGRAPH(notebook) pages] $CGRAPH(current_page)]+1]]
    }
    bind . <Next> {
	$CGRAPH(notebook) raise [lindex [$CGRAPH(notebook) pages] [expr [lsearch [$CGRAPH(notebook) pages] $CGRAPH(current_page)]-1]]
    }
}

proc newTab { {title ""} } {
    set ::CGRAPH(current_page) cgraph[incr ::CGRAPH(counter)]
    if {$title == ""} {
	set title $::CGRAPH(current_page)
    }
    set nbf [$::CGRAPH(notebook) insert end $::CGRAPH(current_page) -text $title \
		 -raisecmd "raiseTab $::CGRAPH(current_page)"]
    # keep track of the cgwin for this page
    $nbf configure -background black
    set ::CGRAPH(current_cgwin) [set ::CGRAPH(${::CGRAPH(current_page)}_cgwin) \
				     [cgwin $nbf.cgraph -background $::CGRAPH(background) \
					  -width $::CGRAPH(width) -height $::CGRAPH(height) \
					  -dbl 1]]
    pack $::CGRAPH(current_cgwin) -fill both -expand true
    # newTabs always go on top and become the active page
    $::CGRAPH(notebook) raise $::CGRAPH(current_page)
    if { $::CGRAPH(background) != "black" } { setcolor $::colors(black) }
}

proc raiseTab { page } {
    set ::CGRAPH(current_page) $page
    set ::CGRAPH(current_cgwin) $::CGRAPH(${page}_cgwin)
    $::CGRAPH(notebook) see $page; # make sure we can see the tab of current window
}

proc closeTab { page } {
    set idx [lsearch [$::CGRAPH(notebook) pages] $page]
    $::CGRAPH(notebook) delete $page
    # clean up a little bit
    unset ::CGRAPH(${page}_cgwin)
    if ![llength [$::CGRAPH(notebook) pages]] {
	# if this was the only tab, then create a new main window
	set ::CGRAPH(counter) -1; # reset counter since we are back to scratch
	newTab "Main"
    } else {
	# try to raise the next highest page (or if this was on top, the one below it)
	if {$idx == [llength [$::CGRAPH(notebook) pages]]} {
	    set idx end
	}
	$::CGRAPH(notebook) raise [lindex [$::CGRAPH(notebook) pages] $idx]
    }
}

proc closeAllTabs { } {
    foreach p [$::CGRAPH(notebook) pages] {
	closeTab $p
    }
}
	       

proc newWindow { { name Figure } { width 640 } { height 480 } { background black } } {
    global counter colors
    set w ".figure$counter"
    incr counter

    toplevel $w
    wm title $w $name

    frame $w.tb -bd 1 -relief raised
    button $w.tb.print -text "Print" -command {dumpwin printer}
    button $w.tb.close -text "Close" -command "destroy $w; $::CGRAPH(current_cgwin) select"
    pack $w.tb.print -side right -padx 10
    pack $w.tb.close -side right -padx 10
    pack $w.tb -fill x
    pack [cgwin $w.cgraph -background $background -width $width \
	    -height $height] -fill both -expand true
    if { $background != "black" } { setcolor $colors(black) }
    return "$w.cgraph"
}


proc loadCGfile file {
	source $file
	flushwin
	focus .
}

proc printCGfile file {
	dumpwin postscript $file
}

proc dump_to_printer {} {
    set f [dl_tempname].pdf
    dumpwin pdf $f
    if [info exists ::acrobat_path] {
	if [file exists $::acrobat_path] {
##	    exec c:/Program\ Files/adobe/adobe\ acrobat\ 6.0/acrobat/acrobat.exe $f &
	    exec $::acrobat_path $f &
	} else {
	    error "dump_to_printer (in dlshell.tcl) - acrobat_path $acrobat_path not valid.\n   set acrobat_path in dlsh.cfg"
	}
    }
}

proc close_cgsh {} {
	exit
}

proc quit {} { close_cgsh }

proc toggle_console {} {
    if { $::console_visible } { 
	console hide
    } else {
	console show
    }
    return

    package require tkcon
    set ::tkcon::OPT(exec) ""
    set ::tkcon::PRIV(root) .console  ;# optional---defaults to .tkcon
    tkcon show
    wm protocol .console WM_DELETE_WINDOW { exit }
}

configureMainWindow

