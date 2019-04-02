default: simpsh.c
	gcc -g -Wall -Wextra simpsh.c -o simpsh
clean:
	rm -f simpsh
check: testInvalidCommand testVerbose testMultArg

testInvalidCommand:
		echo | ./simpsh --wrongCommand >check1out.txt 2>check1err.txt; \
		if [[ $$? == 1 ]] && [ ! -s check1out.txt ] && [ -s check1err.txt ]; \
		then \
			echo "Passed Check 1!"; \
		else \
			echo "Failed Check 1";\
		fi ;\
		rm -f check1out.txt check1err.txt; \

testVerbose:
		echo | ./simpsh --wronly test2out.txt --wronly test3out.txt --verbose --wronly test4out.txt --command 1 0 2 cat>c2out.txt 2>c2err.txt; \
		if  grep -q -- "--wronly test4out" c2out.txt ; \
		then \
			echo "Passed Check 2!";\
		else \
			echo "Failed Check 2"; \
		fi ;\
		rm -f test2out.txt test3out.txt test4out.txt c2out.txt c2err.txt ;\

testMultArg:
	echo | touch test3in.txt ; \
	echo | touch test3out.txt ; \
	echo | touch test3err.txt ; \
	echo | ./simpsh --rdonly test3in.txt --wronly test3out.txt --wronly test3err.txt --command 0 1 2 time -p sleep 3 >c3out.txt 2>&1 ; \
		T3RC=$$?; \
		echo | sleep 4; \
		if [[ $$T3RC == 0 ]] && grep -q -- "real 3." test3err.txt ; \
		then \
			echo "Passed Check 3!"; \
		else \
			echo "Failed Check 3"; \
		fi ;\
		rm -f test3in.txt test3out.txt test3err.txt c3out.txt; \

dist:
	tar -cvzf lab1-504923307.tar.gz simpsh.c README Makefile



