#
# NAME
#  lut.tcl
#
# DESCRIPTION
#   Functions for creating lut's that can be used to create images
# 
# AUTHOR
#   DLS, APR 96
#

#
# A real nightmare, I must ad
#
proc lut::coldhot { { m "" } { sd "" } } {
    set lut [dg_create]

    dl_set t [dl_div [dl_series 0 255] 255.]
    set add dg_addExistingList
    $add $lut [dl_char [dl_mult 255 \
	    [dl_add [dl_mult 0.9 [dl_weibull [dl_mult t 10.0] 1.0 4.5 4.0]] \
	    [dl_combine [dl_zeros 185.] [dl_series 0 .1 [expr 0.1/70.0]]]]]] r
    $add $lut [dl_char [dl_mult 255 [dl_gaussian t 1.0 .43 0.29 ]]] g
    $add $lut [dl_char [dl_mult 255 [dl_gaussian t 1.0 .08 0.15]]]  b
    return $lut
}

proc lut::delete { lut } {
    dg_delete $lut
}
