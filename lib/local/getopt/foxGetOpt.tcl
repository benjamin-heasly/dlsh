#!/bin/false -- do not execute!
#
# getopt -- a getopt(3)-like routine to parse command line options
#
# usage:
# getopt <args-list> <opt-string> <opt-ret> <arg-ret>
#
# getopt sets the variable <opt-ret> to be the next option letter in
# <args-list> that matches a letter in <opt-string>.  <opt-string> must
# contain the option letters the command using getopt will recognize;
# if a letter is followed by a colon, the option is expected to have an
# argument.  The variable <arg-ret> will be set to the option
# argument, if any.
#
# getopt sets the global variables optind and optindc to point to the
# next option letter in <args-list> to be processed; optind holds the
# index of the option in the list, and optindc holds the index of the
# option letter in the string.
#
# When all options have been processed (that is, up to the first
# non-option argument), getopt returns an empty string.  The special
# option ``--'' may be used to delimit the end of the options; when it
# is encountered, <opt-ret> will be set to the empty string, and the
# ``--'' will be skipped.
#
# If getopt encounters an option in <args-list> that is not described
# in <opt-string>, or it finds an option with no argument when the
# option requires one, it sets <opt-ret> to a blank string and
# <arg-ret> to an error message.
#
# getopt returns 1 if an option was found, 0 if no more options were
# found, and -1 if an error occurred.
#
# EXAMPLE:
#
# the following script accepts the exclusive options -a and -b, and
# the option -o with an argument.
#
# #!/usr/local/bin/tclsh
#
# set opts(a) 0
# set opts(b) 0
# set opts(o) ""
#
# proc usage {} {
#   puts stderr "Usage: $argv0 [ -a | -b ] [ -o <string> ]"
#   exit 22
# }
#
# while { [ set err [ getopt $argv "abo:" opt arg ]] } {
#   if { $err < 0 } then {
#     puts stderr "$argv0: $arg"
#     usage
#   } else {
#     switch -exact $opt {
#       a {
#         if { $found(b) } then {
#           puts stderr "$argv0: Only one of -a and -b may be specified!"
#           usage
#         } else {
#           set found(a) 1
#         }
#       }
#       b {
#         if { $found(a) } then {
#           puts stderr "$argv0: Only one of -a and -b may be specified!"
#           usage
#         } else {
#           set found(b) 1
#         }
#       }
#       o {
#         set found(o) $optarg
#       }
#     }
#   }
# }
#
# set argv [ lrange $argv $optind end ]
#
# if { $found(a) } then {
#   puts stdout "Found option -a"
# }
# if { $found(b) } then {
#   puts stdout "Found option -b"
# }
# if { [ string length $found(o) ] } then {
#   puts stdout "Found option -o: \"$found(o)\""
# }
# puts -nonewline stdout "The rest of the arguments are: "
# set prefix ""
# foreach arg $argv {
#   puts -nonewline stdout "$prefix\"$arg\""
#   set prefix ", "
# }
# puts stdout ""
#
# REVISION HISTORY
#
# $Revision: 1.0 $
#   $Author: darkfox $
#     $Date: 1994/02/19 18:49:40 $
#
# $Log: foxGetOpt.tcl,v $
#   Revision 1.0  1994/02/19  18:49:40  darkfox
#   Initial revision
#

set optind 0
set optindc 0

proc getopt { argslist optstring optret argret } {
  global optind optindc
  upvar $optret retvar
  upvar $argret optarg

# default settings for a normal return
  set optarg ""
  set retvar ""
  set retval 0

# check if we're past the end of the args list
  if { $optind < [ llength $argslist ] } then {

# if we got -- or an option that doesn't begin with -, return (skipping
# the --).  otherwise process the option arg.
    switch -glob -- [ set arg [ lindex $argslist $optind ]] {
      "--" {
        incr optind
      }

      "-*" {
        if { $optindc < 1 } then {
          set optindc 1
        }

        set opt [ string index $arg $optindc ]

        if { [ incr optindc ] == [ string length $arg ] } then {
          set arg [ lindex $argslist [ incr optind ]]
          set optindc 0
        }

        if { [ string match "*$opt*" $optstring ] } then {
          set retvar $opt
          set retval 1
          if { [ string match "*$opt:*" $optstring ] } then {
            if { $optind < [ llength $argslist ] } then {
              set optarg [ string range $arg $optindc end ]
              incr optind
              set optindc 0
            } else {
              set optarg "Option requires an argument -- $opt"
              set retvar $optarg
              set retval -1
            }
          }
        } else {
          set optarg "Illegal option -- $opt"
          set retvar $optarg
          set retval -1
        }
      }
    }
  }

  return $retval
}
