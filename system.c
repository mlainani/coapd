#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

char *myname;

int main(int argc, char *argv[])
{
     char *file = "/proc/uptime";
     int fd;
     char buffer[BUFSIZ];
     ssize_t rcount, wcount;
     int errors = 0;

     myname = argv[0];

     if ((fd = open(file, O_RDONLY)) < 0) {
	  fprintf(stderr, "%s: %s: cannot open for reading: %s\n",
		  myname, file, strerror(errno));
	  return 1;
     }

     while ((rcount = read(fd, buffer, sizeof buffer)) > 0) {
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
