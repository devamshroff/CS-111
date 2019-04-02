#NAME: Devam Shroff
#EMAIL: dev.shroff12@gmail.com
#ID: 504923307

.SILENT:

default:
	gcc -g -Wall -Wextra -o lab2_list lab2_list.c SortedList.c -pthread -lprofiler


tests: default
	touch lab2b_list.csv
	chmod +x test.sh
	./test.sh

graphs:
	./lab2.gp

profile: default
	-rm -f ./raw.gperf
	LD_PRELOAD=/usr/local/cs/gperftools-2.7/lib/libprofiler.so.0
	CPUPROFILE=./raw.gperf ./lab2_list --threads=12 --iterations=1000 --sync=s
	pprof --text ./lab2_list ./raw.gperf > profile.out
	pprof --list=run_thread ./lab2_list ./raw.gperf >> profile.out
	rm -f ./raw.gperf

dist: tests graphs profile
	tar -czf lab2b-504923307.tar.gz SortedList.h SortedList.c test.sh lab2.gp lab2_list.c Makefile lab2b_list.csv profile.out lab2b_1.png lab2b_2.png lab2b_3.png lab2b_4.png lab2b_5.png README

clean:
	rm -f lab2_list lab2b-504923307.tar.gz
