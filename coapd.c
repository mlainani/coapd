#include "coap.h"

#define MAX_LEN	500

char buf[MAX_LEN];

void alarm_handler(int signo)
{
}

int coap_header(uint8_t *hdr, uint8_t type, uint8_t tklen,
		uint8_t code, uint16_t mid)
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
     *(uint16_t *)(hdr + 2) = htons(mid);
     printf("%s: 0x%.8x\n", __func__, *(uint32_t *)hdr);
     
     return 0;
}

int main(int argc, char **argv)
{
     struct sockaddr_in6 servaddr;
     int sockfd, ret, n;
     uint8_t *hdr;
     uint16_t mid;
     struct itimerval timeout;
     socklen_t len;

     if (argc != 2) {
	  printf("usage: client <server ipv6 addr>\n");
	  exit(-1);
     }

     bzero(&servaddr, sizeof(servaddr));
     servaddr.sin6_family = AF_INET6;
     ret = inet_pton(AF_INET6, argv[1], &servaddr.sin6_addr);
     if (ret == 0) {
	  printf("not a valid IPv6 address\n");
	  exit(-1);
     }
     else if (ret < 0) {
	  perror("inet_pton");
	  exit(-1);
     }
     servaddr.sin6_port = htons(COAP_DEFAULT_PORT);

     sockfd = socket(AF_INET6, SOCK_DGRAM, 0);
     if (sockfd < 0) {
	  perror("socket");
	  exit(-1);
     }

     if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr_in6)) < 0) {
	  perror("connect");
	  close(sockfd);
	  exit(-1);
     }

     /* len = COAP_HDR_SIZE; */
     hdr = malloc(COAP_HDR_SIZE);

     if (!hdr) {
	  printf("can't alloc memory!");
	  close(sockfd);
	  exit(-1);
     }

     srand(time(NULL));

     mid = rand() % 0xffff;
     coap_header(hdr, COAP_MSG_TYPE_ACK, 0x8, COAP_CODE_GET, mid);

     signal(SIGALRM, alarm_handler);

     n = 1000000 * (COAP_ACK_TIMEOUT * (COAP_ACK_RANDOM_FACTOR - 1));

     memset(&timeout, 0, sizeof(timeout));
     timeout.it_value.tv_sec = COAP_ACK_TIMEOUT;
     timeout.it_value.tv_usec = rand() % n;

     if (sendto(sockfd, hdr, COAP_HDR_SIZE, 0, (struct sockaddr *)&servaddr,
		sizeof(servaddr)) < 0) {
	  perror("sendto");
	  free(hdr);
	  close(sockfd);
	  exit(-1);
     }

     ret = setitimer(ITIMER_REAL, &timeout, NULL);
     if (ret) {
	  perror("setitimer");
	  free(hdr);
	  close(sockfd);
	  exit(-1);
     }

     len = sizeof(servaddr);
     for (;;) {
	  n = recvfrom(sockfd, buf, MAX_LEN, 0, (struct sockaddr *)&servaddr, &len);
	  if (n < 0) {
	       perror("recvfrom");
	       exit(-1);
	  }
	  else
	       printf("received %d bytes\n", n);
     }

     free(hdr);
     close(sockfd);
     exit(0);
}
