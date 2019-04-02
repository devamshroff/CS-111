truncate -s 0 lab2_list.csv
iter=( 1 10 100 1000 )
thread=( 2 4 8 12 )
for i in "${thread[@]}"; do
    for j in "${iter[@]}"; do
        ./lab2_list --iterations=$j --threads=$i >> lab2_list.csv
    done
done

















































