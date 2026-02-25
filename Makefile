all:
	@gcc -std=c99 -Wall -pedantic visualiser.c -o visualiser -lc -lasound
	@strip -s visualiser
	@./visualiser plug:hw:2 plug:hw:2

clean:
	@rm -rf visualiser *.o
