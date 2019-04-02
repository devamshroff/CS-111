default: simpsh.c
	gcc -g -Wall -Wextra simpsh.c -o simpsh

clean:
	rm -f simpsh lab1-504923307.tar.gz

check: testFlag testPipe catchAbort

testFlag:
	echo | touch a.txt; \
	echo | touch test1.txt; \
	echo | cp a.txt test1.txt; \
	./simpsh --creat --excl --wronly test1.txt >c1.txt 2>c1err.txt ;\
	if [[ $$? == 1 ]] && [[ ! -s c1.txt ]] && [[ -s c1err.txt ]] ; \
	then \
	echo "test case 1 passed"; \
	else \
	 echo "test case 1 failed"; \
	fi ;\
	rm -f test1.txt c1.txt c1err.txt a.txt;\

testPipe:
	echo | touch test2out.txt;\
	echo | touch test2err.txt;\
	echo | touch a.txt;\
	./simpsh --rdonly a.txt --wronly test2err.txt --pipe --wronly test2out.txt --command 0 3 1 cat --command 2 4 1 cat >c2out.txt 2>&1 ;\
	DS=$$? ;\
	echo | sleep 1 ;\
	if [[ $$DS == 0 ]] && cmp -s a.txt test2out.txt && [[ ! -s test2err.txt ]] && [[ ! -s c2out.txt ]] ;\
	then \
	echo "test case 2 passed";\
	else \
	echo "test case 2 failed";\
	fi;\

catchAbort:
	./simpsh --catch 11 --abort >c3out.txt 2>c3err.txt ;\
	if [[ $$? == 11 ]] && grep -q "11 caught" c3err.txt && [[ ! -s c3out.txt ]] ;\
	then \
	echo "test case 3 passed" ;\
	else \
	echo "test case 3 failed" ;\
	fi \

dist:
	tar -cvzf lab1-504923307.tar.gz simpsh.c README Makefile



