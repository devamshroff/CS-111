rvdefault: lab0.c
	gcc -g -Wall -Wextra lab0.c -o lab0

check: exitTest argTest equalityTest segTest existTest

exitTest:
	echo | ./lab0 --input="file1" --output="file2"; \
	if [[ $$? -eq 0 ]]; \
	then \
		echo "Program successfully exited with exit code 0"; \
	else \
		echo "Program exited with wrong exit code"; \
	fi 
argTest:
	echo | ./lab0 --notAnArg &> /dev/null; \
	if [[ $$? -eq 1 ]]; \
	then \
	echo "Passed. Correct error code for wrong argment."; \
	else \
	echo "Failed. Wrong error code for wrong argument."; \
	fi
equalityTest:
	touch fin; \
	echo | "Hi, my name is Devam" > fin; \
	echo | ./lab0 --input="fin" --output="fout"; \
	cmp fin fout ; \
	if [[ $$? -eq 0 ]]; \
	then \
		echo "Passed. Infile matches outfile."; \
	else \
		echo "Failed. Infile does not match outfile."; \
	fi 
	rm fin fout;
segTest:
	echo | ./lab0 --catch --segfault ; \
	if [[ $$? -eq 4 ]]; \
	then \
		echo "Passed. Segmentation fault created and caught by handler"; \
	else \
		echo "Failed. Either creation or catching of segmentation fault failed."; \
	fi
existTest:
	echo | ./lab0 --input=wrongfile; \
	if [[ $$? -eq 2 ]]; \
	then \
		echo "Passed. Input file could not be read because it does not exist."; \
	else \
	echo "Failed. Different exit code used. Correct code should be 2."; \
	fi 	

clean:
	rm -f lab0 lab0-504923307.tar.gz

dist:
	tar -cvzf lab0-504923307.tar.gz lab0.c README Makefile backtrace.png breakpoint.png
