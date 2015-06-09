#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "defs.h"

enum {
	DUMP_PREFIX_NONE,
	DUMP_PREFIX_ADDRESS,
	DUMP_PREFIX_OFFSET
};

extern void print_hex_dump_bytes(const char *prefix_str, int prefix_type,
				 const void *buf, size_t len);
char *myname;

int sys_uptime_get(char *val, ssize_t *lenp)
{
     int fd;
     ssize_t count;
     char buf[COAP_MSG_MAX_PAYLOAD];

     if (lenp == NULL || *lenp <= 0 || *lenp > COAP_MSG_MAX_PAYLOAD)
	  return -1;
	  
     if ((fd = open("/proc/uptime", O_RDONLY)) < 0)
	  return -1;

     if (val == NULL) {
	  count = read(fd, buf, sizeof(buf));
	  /* Don't include trailing newline character */
	  if (count > 1) {
	       *lenp = count -1 ;
	       return 0;
	  }
     }
     else {
	  count = read(fd, val, *lenp);
	  if (count > 0)
	       return 0;
     }

     close(fd);

     return -1;
}

#if 1
int main(int argc, char *argv[])
{
     int ret;
     ssize_t len;
     char buf[COAP_MSG_MAX_PAYLOAD];
 
     len = sizeof(buf);
     ret = sys_uptime_get(NULL, &len);

     if (ret == -1)
	  return -1;

     if (len <= sizeof(buf))
	       ret = sys_uptime_get(buf, &len);
     else {
	  printf("error: max msg payload size exceeded\n");
	  return -1;
     }

     if (ret == -1) {
	  printf("error: read failed\n");
	  return -1;
     }

     print_hex_dump_bytes("", DUMP_PREFIX_OFFSET, buf, len);

     return 0;
}
#else
int main(int argc, char *argv[])
{
     char *file = "/proc/uptime";
     int fd;
     char buffer[BUFSIZ];
     ssize_t rcount, wcount, count;
     int errors = 0;

     myname = argv[0];

     if ((fd = open(file, O_RDONLY)) < 0) {
	  fprintf(stderr, "%s: %s: cannot open for reading: %s\n",
		  myname, file, strerror(errno));
	  return 1;
     }

     while ((rcount = read(fd, buffer, sizeof buffer)) > 0) {
	  count += rcount;
	  wcount = write(1, buffer, rcount);
	  if (wcount != rcount) {
	       fprintf(stderr, "%s: %s: write error: %s\n",
		       myname, file, strerror(errno));
	       errors++;
	       break;
	  }
     }

     if (rcount < 0) {
	  fprintf(stderr, "%s: %s: read error: %s\n",
		  myname, file, strerror(errno));
	  errors++;
     }

     if (fd != 0) {
	  if (close(fd) < 0) {
	       fprintf(stderr, "%s: %s: close error: %s\n",
		       myname, file, strerror(errno));
	       errors++;
	  }
     }
     return (errors != 0);
}
#endif
