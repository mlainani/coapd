.PHONY: coapd clean

CFLAGS = -Wall -Werror

all: coapd

coapd:
	rm -f coapd
	$(CC) $(CFLAGS) coapd.c -o coapd

clean:
	rm -f coapd
