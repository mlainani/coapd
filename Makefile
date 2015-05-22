.PHONY: client coapd cscope list dummy clean

CFLAGS = -Wall -Werror

all: client coapd cscope dummy list

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
	@ cscope -b

dummy:
	rm -f dummy
	$(CC) $(CFLAGS) dummy.c -o dummy

# indent:
# 	indent -kr foobar.c

list:
	rm -f list
	$(CC) $(CFLAGS) list.c -o list

clean:
	rm -f client coapd cscope.* dummy list patricia
