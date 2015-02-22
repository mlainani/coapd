.PHONY: coapd

CFLAGS = -Wall -Werror

coapd:
	rm -f coapd
	gcc ${CFLAGS} coapd.c -o coapd
