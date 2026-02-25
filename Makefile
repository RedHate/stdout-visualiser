all:
	@gcc -std=c99 -Wall -pedantic visualiser.c -o visualiser -lc -lasound
	@strip -s visualiser

clean:
	@rm -rf visualiser *.o
