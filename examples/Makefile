.PHONY: clean hash

CFLAGS = -g -Wall -Werror

clean:
	rm -f graph hash *.gv *.png *.svg *~

hash: eprintf.c hash.c
	gcc -o hash eprintf.c hash.c
