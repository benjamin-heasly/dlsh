#
# NAME
#    img_show.tcl
#
# DESCRIPTION
#    Provide utilities for image loading/showing
#
#  ** Note that the "expect" library is loaded (once) to provide the
#  ** spawn function. 
#

set expect_lib "/usr/local/lib/libexpect5.20.so Expect"
if {[lsearch [info loaded] $expect_lib] == -1} {
    eval load $expect_lib
}

###########################################################################
#
#  PROC
#    img_show
#
#  DESCRIPTION
#    Spawn a "display" process that reads from stdin and then
#  write the image to stdout
#
###########################################################################

proc img_show { img } {
    global spawn_id
    
    if {[img_depth $img] == 3} {
	set FORMAT "RGB"
    } else {
	set FORMAT "GRAY"
    }
    set CMDLINE "|display -title $img -size [img_dims $img] $FORMAT:-"
    spawn -noecho -leaveopen [open $CMDLINE w]
    set f [exp_open -i $spawn_id]
    img_write -channel $f $img
    close $f
}

###########################################################################
#
#  PROC
#    gd_show
#
#  DESCRIPTION
#    Spawn a "display" process that reads from stdin and then
#  write the image to stdout using the gd libarary
#
###########################################################################

proc gd_show { img } {
    global spawn_id

    set CMDLINE "|display -title $img -"
    spawn -noecho -leaveopen [open $CMDLINE w]
    set f [exp_open -i $spawn_id]
    gd writeGIF $f $img
    close $f
}

