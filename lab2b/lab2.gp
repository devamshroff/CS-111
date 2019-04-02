#! /usr/local/cs/bin/gnuplot

# NAME: Devam Shroff
# EMAIL: dev.shroff12@gmail.com
# ID: 504923307

# general plot parameters
set terminal png
set datafile separator ","

# lab2b_1.png
set title "1: Throughput of Synchronized Lists"
set xlabel "Threads"
set logscale x 2
unset xrange
set xrange [0.75:]
set ylabel "Throughput (operations/sec)"
set logscale y
set output 'lab2b_1.png'
set key left top
plot \
     "< grep -e 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'list operations w/mutex' with linespoints lc rgb 'blue', \
     "< grep -e 'list-none-s,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'list operations w/spin-lock' with linespoints lc rgb 'green'


# lab2b_2.png
set title "2: Per-operation Times for Protected List Operations"
set xlabel "Threads"
set logscale x 10
unset xrange
set xrange [1:]
set ylabel "Mean Time/Operation (ns)"
set logscale y
set output 'lab2b_2.png'
set key left top
plot \
     "< grep -e 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):($8) \
	title 'wait for lock' with linespoints lc rgb 'blue', \
     "< grep -e 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):($7) \
	title 'completion time' with linespoints lc rgb 'red'


# lab2b_3.png
set title "3: Correct Synchronization of Partitioned Lists"
set xlabel "Threads"
set logscale x 2
unset xrange
set xrange [0.75:]
set ylabel "Successful Iterations"
set logscale y
set output 'lab2b_3.png'
set key left top
plot \
    "< grep list-id-none lab2b_list.csv" using ($2):($3) \
	title 'Unprotected' with points lc rgb 'red', \
     "< grep list-id-m lab2b_list.csv" using ($2):($3) \
	title 'Mutex' with points lc rgb 'orange', \
     "< grep list-id-s lab2b_list.csv" using ($2):($3) \
	title 'Spin-Lock' with points lc rgb 'yellow'


# lab2b_4.png
set title "4: Throughput of Mutex-Synchronized Partitioned Lists"
set xlabel "Threads"
set logscale x 2
unset xrange
set xrange [0.75:]
set ylabel "Throughput (operations/sec)"
set logscale y
set output 'lab2b_4.png'
set key left top
plot \
    "< grep -E 'list-none-m,[0-9],1000,1,|list-none-m,12,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'lists=1' with linespoints lc rgb 'red', \
    "< grep -E 'list-none-m,[0-9],1000,4,|list-none-m,12,1000,4,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'lists=4' with linespoints lc rgb 'blue', \
    "< grep -E 'list-none-m,[0-9],1000,8,|list-none-m,12,1000,8,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'lists=8' with linespoints lc rgb 'green', \
    "< grep -E 'list-none-m,[0-9],1000,16,|list-none-m,12,1000,16,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'lists=16' with linespoints lc rgb 'orange'


# lab2b_5.png
set title "5: Throughput of Spin-Lock-Synchronized Partitioned Lists"
set xlabel "Threads"
set logscale x 2
unset xrange
set xrange [0.75:]
set ylabel "Throughput (operations/sec)"
set logscale y
set output 'lab2b_5.png'
set key left top
plot \
    "< grep -E 'list-none-s,[0-9],1000,1,|list-none-s,12,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'lists=1' with linespoints lc rgb 'orange', \
    "< grep -E 'list-none-s,[0-9],1000,4,|list-none-s,12,1000,4,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'lists=4' with linespoints lc rgb 'green', \
    "< grep -E 'list-none-s,[0-9],1000,8,|list-none-s,12,1000,8,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'lists=8' with linespoints lc rgb 'blue', \
    "< grep -E 'list-none-s,[0-9],1000,16,|list-none-s,12,1000,16,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'lists=16' with linespoints lc rgb 'red'
