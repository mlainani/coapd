#include "defs.h"

#include "msgtab.h"
#include "codes.h"
#include "options.h"

static struct server srv;

uint8_t buf[COAP_MSG_MAX_SIZE];

/* 
 * The only problem is that there's no NULL terminator in the Uri-Path
 * options carried in the message
 */
char *uri_path_vec[URI_PATH_VEC_MAX_SIZE];

int sock_init(struct in6_addr *addr)
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

static int bcompar(const void *p1, const void *p2)
{
     struct code *m1 = (struct code *)p1;
     struct code *m2 = (struct code *)p2;
     
     if (m1->val < m2->val)
	  return -1;
     if (m1->val > m2->val)
	  return 1;
     return 0;
}

int send_ack(uint16_t mid)
{
     return 0;
}

int send_reset(uint16_t mid)
{
     struct msg *msgp;
     uint8_t hdr[COAP_HDR_SIZE] = { 0 };
     struct sockaddr_in6 dst;
     int ret = -1;

     msgp = msgtab_lookup(mid, NULL, false);
     
     if (msgp != NULL) {

	  bzero(&dst, sizeof(dst));
	  dst.sin6_family = AF_INET6;
	  memcpy(&dst.sin6_addr, &msgp->src.sin6_addr, sizeof(struct in6_addr));
	  dst.sin6_port = htons(COAP_DEFAULT_PORT);

	  hdr[0] |= COAP_VERSION_BITS;
	  hdr[0] |= COAP_TYPE_RST_BITS;
	  *(uint16_t *)&hdr[2] = htons(mid);

	  ret = sendto(srv.sockfd, hdr, COAP_HDR_SIZE, 0,
		       (struct sockaddr *)&dst, sizeof(dst));
	  if (ret < 0)
	       handle_error("sendto");
     }

     return ret;
}

int send_empty(int sockfd, uint16_t mid)
{
     return 0;
}

static inline const char *type2str(uint8_t type)
{
     return type_str[type];
}

int parse(uint8_t *hdr, size_t len, struct sockaddr_in6 *src)
{
     uint8_t version, type, tklen;
     struct code key, *code;
     uint16_t mid;
     int ret = -1;

     version = coap_hdr_ver(*hdr);
     if (version != COAP_VERSION) {
	  printf("%s: msg ignored (unknown version num %d)\n",
		 __func__, version);
	  goto out;
     }

     type = coap_hdr_type(*hdr);

     tklen = coap_hdr_tklen(*hdr);
     if (tklen > 8) {
	  printf("%s: msg format error (tklen %d)\n",
		 __func__, tklen);
	  goto out;
     }

     key.val = *(hdr + 1);
     code = bsearch(&key, codes, nr_of_codes,
		   sizeof(struct code), bcompar);
     if (code == NULL) {
	  printf("%s: msg ignored (unknown code %d)\n",
		 __func__, code->val);
	  goto out;
     }

     mid = htons(*(uint16_t *)(hdr + 2));

     printf("v: %d  type: %s  tklen: %d  code: %s  mid: %d\n",
	    version, type2str(type), tklen, code->name, mid);

     /* Add new entry to the hash table */
     msgtab_lookup(mid, src, true);
     
     if (len > COAP_HDR_SIZE + tklen) {
	  /* Options are present */
	  ret = parse_options(hdr + COAP_HDR_SIZE + tklen,
			      len - COAP_HDR_SIZE - tklen,
			      code->val);
     }
     else {
	  /* "CoAP ping" */
	  if (type == COAP_TYPE_CON && code == COAP_EMPTY)
	       send_reset(mid);
	  goto out;
     }

     if (code->handler != NULL)
	  ret = code->handler(mid);
out:
     return ret;
}

int main(int argc, char **argv)
{
     struct sockaddr_in6 cliaddr;
     socklen_t addrlen;
     int n;

     /* Server listening socket */
     if (sock_init(NULL) == -1)
	  exit(EXIT_FAILURE);

     /* Initialize client messages hash table  */
     msgtab_init();

     addrlen = sizeof(struct sockaddr_in6);

     /* Wait for incoming messages */
     for (;;) {
	  n = recvfrom(srv.sockfd, buf, COAP_MSG_MAX_SIZE, 0,
		       (struct sockaddr *)&cliaddr, &addrlen);
	  if (n < 0) {
	       perror("recvfrom");
	  }
	  else {
	       parse((uint8_t *)buf, n, &cliaddr);
	       /* continue; */
	  }
	  msgtab_walk();
     }

     close(srv.sockfd);
     exit(EXIT_SUCCESS);
}
