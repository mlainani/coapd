#include <search.h>
#include <stdlib.h>

#include "defs.h"

void *root = NULL;

void alarm_handler(int signo)
{
}

int coap_header(uint8_t *hdr, uint8_t type, uint8_t tklen,
		uint8_t code, uint16_t id)
{
     if (tklen > 8) {
	  printf("invalid token length %d\n", tklen);
	  return -1;
     }

     *hdr = 0;
     *hdr |= COAP_VERSION << 6;
     *hdr |= type << 4;
     *hdr |= tklen;
     *(hdr + 1) = code;
     *(uint16_t *)(hdr + 2) = htons(id);
     printf("%s: 0x%.8x\n", __func__, *(uint32_t *)hdr);
     
     return 0;
}

static int comparname(const void *p1, const void *p2)
{
     option *opt1 = (option *)p1;
     option *opt2 = (option *)p2;

     return strcmp(opt1->name, opt2->name);
}

static void init()
{
     int i;
     void *val;

     for (i = 0; i < NOPTS; i++) {
	  val = tsearch((void *)&options[i], &root, comparname);
	  if (val == NULL)
	       exit(EXIT_FAILURE);
	  else
	       printf("Added %s to the tree\n", options[i].name);
     }
}

/* 
 * The first field in each node of the tree is a pointer to the
 * corresponding data item. (The calling program must store the actual
 * data.)
 */

static void action(const void *nodep, const VISIT which, const int depth)
{
     option *opt;

     switch (which) {
     case preorder:			/* before visiting the  children */
	  break;
     case postorder:			/* after the 1st and before the 2nd */
	  opt = *(option **)nodep;
	  printf("%s\n", opt->name);
	  break;
     case endorder:			/* after visiting the children */
	  break;
     case leaf:
	  opt = *(option **)nodep;
	  printf("%s\n", opt->name);
	  break;
     }
}

static int comparnum(const void *p1, const void *p2)
{
     option *opt1 = (option *)p1;
     option *opt2 = (option *)p2;

     if (opt1->num < opt2->num)
	  return -1;
     if (opt1->num > opt2->num)
	  return 1;
     return 0;
}

int main(int argc, char **argv)
{
     struct sockaddr_in6 servaddr;
     int sockfd, ret, n, i;
     uint8_t *hdr;
     uint16_t id;
     struct itimerval timeout;

     if (argc != 2) {
	  printf("usage: client <server ipv6 addr>\n");
	  exit(EXIT_FAILURE);
     }

     bzero(&servaddr, sizeof(servaddr));
     servaddr.sin6_family = AF_INET6;
     ret = inet_pton(AF_INET6, argv[1], &servaddr.sin6_addr);
     if (ret == 0) {
	  printf("not a valid IPv6 address\n");
	  exit(EXIT_FAILURE);
     }
     else if (ret == -1)
	  handle_error("inet_pton");

     servaddr.sin6_port = htons(COAP_DEFAULT_PORT);
     sockfd = socket(AF_INET6, SOCK_DGRAM, 0);
     if (sockfd == -1)
	  handle_error("socket");

     if (connect(sockfd, (struct sockaddr *)&servaddr,
		 sizeof(servaddr)) == -1) {
	  close(sockfd);
	  handle_error("connect");
     }

     hdr = malloc(COAP_HDR_SIZE);

     if (!hdr) {
	  printf("can't alloc memory!");
	  close(sockfd);
	  exit(EXIT_FAILURE);
     }

     srand(time(NULL));

     id = rand() % 0xffff;
     coap_header(hdr, COAP_MSG_TYPE_ACK, 0x8, COAP_CODE_GET, id);

     signal(SIGALRM, alarm_handler);

     n = 1000000 * (COAP_ACK_TIMEOUT * (COAP_ACK_RANDOM_FACTOR - 1));

     memset(&timeout, 0, sizeof(timeout));
     timeout.it_value.tv_sec = COAP_ACK_TIMEOUT;
     timeout.it_value.tv_usec = rand() % n;

     if (sendto(sockfd, hdr, COAP_HDR_SIZE, 0, (struct sockaddr *)&servaddr,
		sizeof(servaddr)) == -1) {
	  free(hdr);
	  close(sockfd);
	  handle_error("sendto");
     }

     ret = setitimer(ITIMER_REAL, &timeout, NULL);
     if (ret) {
	  free(hdr);
	  close(sockfd);
	  handle_error("setitimer");
     }

     /* Array binary search by option number */
     for (i = 1; i < argc; i++) {
	  option key, *res;
	  key.num = atoi(argv[i]);
	  res = bsearch(&key, options, NOPTS,
			sizeof(option), comparnum);
	  if (res == NULL)
	       printf("'%s': unknown option\n", argv[i]);
	  else
	       printf("%s: option #%d\n", res->name, res->num);
     }

     /* Binary tree search by option name */
     init();
     twalk(root, action);

     free(hdr);
     close(sockfd);
     exit(EXIT_SUCCESS);
}
