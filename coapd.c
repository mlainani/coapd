#include "defs.h"

#include "msgtab.h"

static struct server srv;

uint8_t buf[COAP_MSG_MAX_SIZE];

/* 
 * The only problem is that there's no NULL terminator in the Uri-Path
 * options carried in the message
 */
char *uri_path_vec[URI_PATH_VEC_MAX_SIZE];

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

int init(struct in6_addr *addr)
{
     struct sockaddr_in6 sockaddr;

     srv.sockfd = socket(AF_INET6, SOCK_DGRAM, 0);
     if (srv.sockfd == -1) {
	  handle_error("socket");
	  return -1;
     }

     bzero(&sockaddr, sizeof(sockaddr));
     sockaddr.sin6_family = AF_INET6;
     sockaddr.sin6_port = htons(COAP_DEFAULT_PORT);
     if (addr)
	  sockaddr.sin6_addr = *addr;
     else
	  sockaddr.sin6_addr = in6addr_any;
     if (bind(srv.sockfd, (struct sockaddr *)&sockaddr,
	      sizeof(sockaddr)) == -1) {
	  handle_error("bind");
	  return -1;
     }
    
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

int send_ack(uint16_t id)
{
     return 0;
}

int send_rst(uint16_t id)
{
     struct msg *msgp;
     uint8_t hdr[COAP_HDR_SIZE] = { 0 };
     struct sockaddr_in6 dst;
     int ret = -1;

     msgp = msgtab_lookup(id, NULL, false);
     
     if (msgp != NULL) {

	  bzero(&dst, sizeof(dst));
	  dst.sin6_family = AF_INET6;
	  memcpy(&dst.sin6_addr, &msgp->src.sin6_addr, sizeof(struct in6_addr));
	  dst.sin6_port = htons(COAP_DEFAULT_PORT);

	  hdr[0] |= COAP_VERSION_BITS;
	  hdr[0] |= COAP_TYPE_RST_BITS;
	  *(uint16_t *)&hdr[2] = htons(id);

	  ret = sendto(srv.sockfd, hdr, COAP_HDR_SIZE, 0,
		       (struct sockaddr *)&dst, sizeof(dst));
	  if (ret < 0)
	       handle_error("sendto");
     }

     return ret;
}

int send_empty(int sockfd, uint16_t id)
{
     return 0;
}

static inline const char *type2str(uint8_t type)
{
     return type_str[type];
}

int parse(uint8_t *hdr, struct sockaddr_in6 *src)
{
     uint8_t version, type, tklen;
     struct ci key, *res;
     uint16_t mid;

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

     mid = htons(*(uint16_t *)(hdr + 2));

     printf("v: %d  type: %s  tklen: %d  code: %s  mid: %d\n",
	    version, type2str(type), tklen, res->name, mid);

     msgtab_lookup(mid, src, true);

     if (type == COAP_TYPE_NON)
	  send_rst(mid);

     return 0;
}

int main(int argc, char **argv)
{
     struct sockaddr_in6 cliaddr;
     socklen_t len;
     int n;

     if (init(NULL) == -1)
	  exit(EXIT_FAILURE);

     msgtab_init();

     len = sizeof(struct sockaddr_in6);
     for (;;) {
	  n = recvfrom(srv.sockfd, buf, COAP_MSG_MAX_SIZE, 0,
		       (struct sockaddr *)&cliaddr, &len);
	  if (n < 0) {
	       perror("recvfrom");
	  }
	  else {
	       parse((uint8_t *)buf, &cliaddr);
	       continue;
	  }
	  msgtab_walk();
     }

     close(srv.sockfd);
     exit(EXIT_SUCCESS);
}
