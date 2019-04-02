#!/bin/bash

declare -a threads=(1 2 4 8 12)


declare -a iterations=(1 2 4 8 16)

declare -a lists=(1 4 8 16)
rm -f lab2b_list.csv

# lab2b_list test data
# lab2b_list-1.png, lab2b_list-2.png, lab2b_list-4.png and lab2b_list-5.png
for i in "${threads[@]}"
do
    for j in "${lists[@]}"
	do
        ./lab2_list --iterations=1000 --threads="$i" --lists="$j" --sync=m >> lab2b_list.csv
        ./lab2_list --iterations=1000 --threads="$i" --lists="$j" --sync=s >> lab2b_list.csv
    done
done

threads=(16 24)
iterations=(10 20 40 80)
for i in "${threads[@]}"
do
    ./lab2_list --iterations=1000 --threads="$i" --sync=m >> lab2b_list.csv
    ./lab2_list --iterations=1000 --threads="$i" --sync=s >> lab2b_list.csv
done

# lab2b_list-3.png
threads=(1 4 8 12 16)
for i in "${threads[@]}"
do
    for j in "${iterations[@]}"
	do
        ./lab2_list --iterations="$j" --threads="$i" --yield=id --lists=4 >> lab2b_list.csv
    done
    for j in "${iterations[@]}"
	do
        ./lab2_list --iterations="$j" --threads="$i" --yield=id --lists=4 --sync=m >> lab2b_list.csv
        ./lab2_list --iterations="$j" --threads="$i" --yield=id --lists=4 --sync=s >> lab2b_list.csv
    done
done