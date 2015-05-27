.PHONY: client coapd cscope list resources dummy clean

CFLAGS = -g -Wall -Werror

all: client coapd cscope dummy list resources

client:
	rm -f client
	$(CC) $(CFLAGS) client.c -o client

coapd: coapd.c msgtab.c eprintf.c
	rm -f coapd
	$(CC) $(CFLAGS) coapd.c msgtab.c eprintf.c -o coapd

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

clean:
	rm -f client coapd cscope.* dummy list resources patricia *~ *.o
