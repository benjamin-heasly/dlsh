#
# PROC
#   mahala.tcl
#
# DETAILS
#   Returns the distance between two sets of uni- or mulitvariate
# populations.
#
#

proc maha::dist { x y } {
    if { [dl_isMatrix $x] != 1 || [dl_isMatrix $x] != 1 } {
	error "maha::dist requires that x and y are matrices"
    }
    if { [dl_length $x:0] != [dl_length $y:0] } {
	error "maha::dist - matrices must have equal numbers of cols"
    }
    set nx [dl_length $x]	;# Number of rows in x
    set ny [dl_length $y]	;# Number of rows in y
    set nu [expr $nx+$ny-2]	;# Divisor for cov(u)

    # Find the covariance matrices for both sets of data
    dl_local cov(x)    [dm_covar $x]
    dl_local cov(y)    [dm_covar $y]

    # Find the sample covariance for both populations
    dl_local cov(u)    [dm_add [dl_mult $nx $cov(x)] [dl_mult $ny $cov(y)]]
    dl_set   $cov(u)   [dl_div $cov(u) $nu]
    dl_local invcov(u) [dm_inverse $cov(u)]

    # Get the means of each population
    dl_local mean(x)   [dl_means [dm_transpose $x]]
    dl_local mean(y)   [dl_means [dm_transpose $y]]

    # Get the difference between each dimension
    dl_local meandiff  [dl_llist [dl_sub $mean(x) $mean(y)]]

    # And finally, calculate the mahalanobis distance
    dl_local mahala [dm_mult [dm_mult $meandiff $invcov(u)] \
	    [dm_transpose $meandiff]]
    return [dl_tcllist $mahala]
}

proc maha::hotelling { x y } {
    set mdist [maha::dist $x $y]
    scan [dm_dims $x] "%d %d" nx p
    set ny [dl_length $y]	;# Number of rows in y
    set n  [expr $nx+$ny]
    set t2 [expr (($nx*$ny*($n-$p-1.0))/($n*($n-2.0)*$p))*$mdist]
    return [format "%6.2f %d %d" $t2 $p [expr $n-$p-1]]
}
