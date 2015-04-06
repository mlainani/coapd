#include <stdlib.h>

#include "defs.h"

char buf[COAP_MSG_MAX_SIZE];

int compar(const void *p1, const void *p2)
{
     option *opt1 = (option *)p1;
     option *opt2 = (option *)p2;

     if (opt1->num < opt2->num)
	  return -1;
     if (opt1->num > opt2->num)
	  return 1;
     return 0;
}

int coap_init()
{
     /* Options array is already sorted by option number */
     /* qsort(options, NOPTS, sizeof(option), compnum); */
     return 0;
}

int main(int argc, char **argv)
{
     struct sockaddr_in6 servaddr;
     int sockfd, n;
     socklen_t len;

     sockfd = socket(AF_INET6, SOCK_DGRAM, 0);
     if (sockfd == -1)
	  handle_error("socket");

     bzero(&servaddr, sizeof(servaddr));
     servaddr.sin6_family = AF_INET6;
     servaddr.sin6_addr = in6addr_any;
     if (bind(sockfd, (struct sockaddr *)&servaddr,
	      sizeof(servaddr)) == -1)
	  handle_error("bind");

     len = sizeof(servaddr);
     for (;;) {
	  n = recvfrom(sockfd, buf, COAP_MSG_MAX_SIZE, 0,
		       (struct sockaddr *)&servaddr, &len);
	  if (n < 0) {
	       perror("recvfrom");
	  }
	  else
	       printf("received %d bytes\n", n);
     }

     close(sockfd);
     exit(EXIT_SUCCESS);
}
