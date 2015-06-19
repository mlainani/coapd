.PHONY: client coapd cscope list resources system dummy clean

CFLAGS = -g -Wall -Werror

all: client coapd cscope dummy list resources system

client:
	rm -f client
	$(CC) $(CFLAGS) client.c -o client

coapd: coapd.c msgtab.c eprintf.c methods.c
	rm -f coapd
	$(CC) $(CFLAGS) coapd.c msgtab.c eprintf.c methods.c -o coapd

cscope: 
	@ rm -f cscope.*
	@ echo -q > cscope.files
	@ find . -type f -name '*.[ch]' >> cscope.files
	@ cscope -b

dummy:
	rm -f dummy
	$(CC) $(CFLAGS) dummy.c -o dummy

# indent:
# 	indent -kr foobar.c

list:
	rm -f list
	$(CC) $(CFLAGS) list.c -o list

resources: resources.c eprintf.c
	rm -f resources
	$(CC) $(CFLAGS) resources.c eprintf.c -o resources

system: hexdump.c system.c
	rm -f system
	$(CC) $(CFLAGS) hexdump.c system.c -o system

clean:
	rm -f client coapd cscope.* dummy list resources system *~ *.o
