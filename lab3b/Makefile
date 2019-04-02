default: lab3b.py
	rm -f lab3b
	chmod +x lab3b.py
	cat lab3b.py >> lab3b
	chmod +x lab3b

dist: default
	tar -czf lab3b-504923307.tar.gz Makefile lab3b.py README

clean:
	rm -f lab3b lab3b-504923307.tar.gz
