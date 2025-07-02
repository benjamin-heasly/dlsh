#
# show a given frame from a bink file
#

package require bink

proc get_frame { binkfile frame } {
    set b [bink open $binkfile]
    bink goto $b $frame
    set pixels [bink getframepng $b]
    bink close $b
    return $pixels
}

proc save_to_png { frame png } {
    set f [open $png wb]
    puts -nonewline $f $frame
    close $f
}

if { $argc < 3 } { 
    error "usage: frame2png.tcl binkfile frame pngfile"
} else {
    lassign $argv binkfile frame pngfile
}

set frame [get_frame $binkfile $frame]
save_to_png $frame $pngfile

