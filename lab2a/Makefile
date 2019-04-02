#NAME: Devam Shroff
#EMAIL: dev.shroff12@gmail.com
#ID: 504923307

.SILENT:

all: build

build: 
	gcc -g -Wall -Wextra -o lab2_add lab2_add.c -pthread 
	gcc -g -Wall -Wextra -o lab2_list lab2_list.c SortedList.c -pthread

dist: tests graphs 
	tar -czf lab2a-504923307.tar.gz lab2_add.c lab2_list.c SortedList.c SortedList.h Makefile test1.sh README lab2_add.csv lab2_list.csv lab2_add.gp.sh lab2_list.gp.sh lab2_add-1.png lab2_add-2.png lab2_add-3.png lab2_add-4.png lab2_add-5.png lab2_list-1.png lab2_list-2.png lab2_list-3.png lab2_list-4.png

tests: build
	touch lab2_add.csv
	touch lab2_list.csv
	chmod +x test1.sh
	./test1.sh

graphs:
	./lab2_add.gp.sh
	./lab2_list.gp.sh

clean:
	rm -f lab2_add lab2_list lab2a-504923307.tar.gz
