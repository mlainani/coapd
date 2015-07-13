#include "defs.h"

#include "msgtab.h"
#include "codes.h"
#include "request.h"

static struct server srv;

uint8_t buf[COAP_MSG_MAX_SIZE];

/* Sorted array of messages for binary search */
static struct message {
     uint8_t code;
     char *name;
     int (*handler)(uint16_t mid, uint8_t *buf, size_t buflen);
} messages[] = {
     { COAP_EMPTY, "Empty" },

     { COAP_REQ_GET, "GET" , coap_req_get },
     { COAP_REQ_POST, "POST" },
     { COAP_REQ_PUT, "PUT" },
     { COAP_REQ_DEL, "DELETE" },

     { COAP_RESP_CREATED, "Created" },
     { COAP_RESP_DELETED, "Deleted" },
     { COAP_RESP_VALID, "Valid" },
     { COAP_RESP_CHANGED, "Changed" },
     { COAP_RESP_CONTENT, "Content" },

     { COAP_RESP_BAD_REQUEST, "Bad Request" },
     { COAP_RESP_UNAUTHORIZED, "Unauthorized" },
     { COAP_RESP_BAD_OPTION, "Bad Option" },
     { COAP_RESP_FORBIDDEN, "Forbidden" },
     { COAP_RESP_NOT_FOUND, "Not Found" },
     { COAP_RESP_METHOD_NOT_ALLOWED, "Method Not Allowed" },
     { COAP_RESP_NOT_ACCEPTABLE, "Not Acceptable" },
     { COAP_RESP_PRECONDITION_FAILED, "Precondition Failed" },
     { COAP_RESP_REQUEST_ENTITY_TOO_LARGE, "Request Entity Too Large" },
     { COAP_RESP_UNSUPPORTED_CONTENT_FORMAT, "Unsupported Content-Format" },

     { COAP_RESP_INTERNAL_SERVER_ERROR, "Internal Server Error" },
     { COAP_RESP_NOT_IMPLEMENTED, "Not Implemented" },
     { COAP_RESP_BAD_GATEWAY, "Bad Gateway" },
     { COAP_RESP_SERVICE_UNAVAILABLE, "Service Unavailable" },
     { COAP_RESP_GATEWAY_TIMEOUT, "Gateway Timeout" },
     { COAP_RESP_PROXYING_NOT_SUPPORTED, "Proxying Not Supported" }
};

#define nr_of_messages (sizeof(messages) / sizeof(messages[0]))


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
     struct message *m1 = (struct message *)p1;
     struct message *m2 = (struct message *)p2;
     
     if (m1->code < m2->code)
	  return -1;
     if (m1->code > m2->code)
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
     struct message key, *msg;
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

     key.code = *(hdr + 1);
     msg = bsearch(&key, messages, nr_of_messages,
		   sizeof(struct message), bcompar);
     if (msg == NULL) {
	  printf("%s: msg ignored (unknown message %d)\n",
		 __func__, msg->code);
	  goto out;
     }

     mid = htons(*(uint16_t *)(hdr + 2));

     printf("v: %d  type: %s  tklen: %d  code: %s  mid: %d\n",
	    version, type2str(type), tklen, msg->name, mid);

     /* Add new entry to the hash table */
     msgtab_lookup(mid, src, true);
     
     if (len > COAP_HDR_SIZE + tklen) {
	  /* Options are present */
#if 0
	  ret = parse_options(hdr + COAP_HDR_SIZE + tklen,
			      len - COAP_HDR_SIZE - tklen,
			      msg->code);
#endif
	  if (msg->handler != NULL)
	       ret = msg->handler(mid,
				  hdr + COAP_HDR_SIZE + tklen,
				  len - COAP_HDR_SIZE - tklen);
     }
     else {
	  /* "CoAP ping" */
	  if (type == COAP_TYPE_CON && msg->code == COAP_EMPTY)
	       send_reset(mid);
	  goto out;
     }

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
