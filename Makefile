.PHONY: client coapd cscope dummy clean

CFLAGS = -Wall -Werror

all: client coapd cscope dummy

client:
	rm -f client
	$(CC) $(CFLAGS) client.c -o client

coapd:
	rm -f coapd
	$(CC) $(CFLAGS) coapd.c -o coapd

cscope: 
	@ rm -f cscope.*
	@ echo -q > cscope.files
	@ find . -type f -name '*.[ch]' >> cscope.files
	@ cscope -b -I/usr/include

dummy:
	rm -f dummy
	$(CC) $(CFLAGS) dummy.c -o dummy

# indent:
# 	indent -kr foobar.c

clean:
	rm -f client coapd cscope.* dummy patricia
