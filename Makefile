.PHONY: client coapd cscope list resources system dummy clean

CFLAGS = -g -Wall -Werror

all: client coapd cscope dummy list resources system

client:
	rm -f client
	$(CC) $(CFLAGS) client.c codes.c -o client

coapd: coapd.c msgtab.c eprintf.c methods.c request.c
	rm -f coapd
	$(CC) $(CFLAGS) coapd.c msgtab.c eprintf.c methods.c request.c -o coapd

cscope: 
	@ rm -f cscope.*
	@ echo -q > cscope.files
	@ find . -type f -name '*.[ch]' >> cscope.files
	@ cscope -b

dummy:
	rm -f dummy
	$(CC) $(CFLAGS) dummy.c -o dummy

etags:
	ls *.[ch] | etags -

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

server: common.c main.c message.c option.c request.c server.c
	rm -f server
	$(CC) $(CFLAGS) common.c main.c message.c option.c request.c server.c -o server -lpthread

test_codes: test_codes.c codes.c
	rm -f test_codes
	$(CC) $(CFLAGS) test_codes.c codes.c -o test_codes

test_options: test_options.c options.c
	rm -f test_options
	$(CC) $(CFLAGS) test_options.c options.c -o test_options

clean:
	rm -f TAGS
	rm -f client coapd cscope.* dummy list resources server system test_codes test_options
	rm -f *~ *.o
