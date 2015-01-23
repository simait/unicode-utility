all:
	gcc -O2 -Wall -pedantic -std=c99 unicode.c unicode-util.c -o unicode-util

clean:
	rm -f *.o unicode-util
