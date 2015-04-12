#include <stdlib.h>

#include "defs.h"

uint8_t buf[COAP_MSG_MAX_SIZE];

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

static int compci(const void *c1, const void *c2)
{
     struct ci *ci1 = (struct ci *)c1;
     struct ci *ci2 = (struct ci *)c2;
     
     if (ci1->code < ci2->code)
	  return -1;
     if (ci1->code > ci2->code)
	  return 1;
     return 0;
}

int parse(uint8_t *hdr)
{
     uint8_t version, type, tklen;
     struct ci key, *res;
     uint16_t id;

     version = coap_hdr_ver(*hdr);
     if (version != COAP_VERSION) {
	  printf("%s: msg ignored (unknown version num %d)\n",
		 __func__, version);
	  return -1;
     }

     type = coap_hdr_type(*hdr);

     tklen = coap_hdr_tklen(*hdr);
     if (tklen > 8) {
	  printf("%s: msg format error (tklen %d)\n",
		 __func__, tklen);
	  return -1;
     }

     key.code = *(hdr + 1);
     res = bsearch(&key, codes, nr_of_codes,
		   sizeof(struct ci), compci);
     if (res == NULL) {
	  printf("%s: msg ignored (unknown code %d)\n",
		 __func__, key.code);
	  return -1;
     }
     else
	  printf("%s: %s msg\n", __func__, res->name);

     id = htons(*(uint16_t *)(hdr + 2));

     printf("v: %d  type: %d  tklen: %d  code: %d  id: %d\n",
	    version, type, tklen, key.code, id);

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
     servaddr.sin6_port = htons(COAP_DEFAULT_PORT);
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
	  else {
	       printf("received %d bytes\n", n);
	       parse((uint8_t *)buf);
	  }
     }

     close(sockfd);
     exit(EXIT_SUCCESS);
}
