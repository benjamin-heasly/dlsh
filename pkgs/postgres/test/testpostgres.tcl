set dlshlib [file join /usr/local dlsh dlsh.zip]
if [file exists $dlshlib] {
    set base [file join [zipfs root] dlsh]
   zipfs mount $dlshlib $base
   set auto_path [linsert $auto_path [set auto_path 0] $base/lib]
}

package require postgres

set conninfo "dbname=mydb user=postgres password=postgres host=localhost port=5432"
set conn [postgres::connect $conninfo]

proc create_table {} {
    set statement {
	DROP TABLE IF EXISTS numbers; CREATE TABLE numbers (id SERIAL PRIMARY KEY, val FLOAT)
    }
    postgres::exec $::conn $statement
}

create_table

set command insert_value
set statement {INSERT INTO numbers (val) VALUES ($1)}
postgres::prepare $conn $command $statement 1

proc addprepared { val } {
    postgres::exec_prepared $::conn $::command $val
}
proc addexec { val } {
    set statement "INSERT INTO numbers (val) VALUES ($val)"
    postgres::exec $::conn $statement
}

for { set i 400 } { $i < 500 } { incr i } { addprepared $i }
for { set i 600 } { $i < 800 } { incr i } { addexec $i }

puts [postgres::query $conn { SELECT val from numbers }]
postgres::finish $conn

