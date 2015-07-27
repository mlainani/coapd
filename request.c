#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

#include "request.h"
#include "eprintf.h"

static struct option options[] = {
     { COAP_OPT_IF_MATCH, true, 0, 8, "If-Match" },
     { COAP_OPT_URI_HOST, false, 1, 255, "Uri-Host" },
     { COAP_OPT_ETAG, true, 1, 8, "ETag" },
     { COAP_OPT_IF_NONE_MATCH, false, 0, 0, "If-None-Match" },
     { COAP_OPT_URI_PORT, false, 0, 2, "Uri-Port" },
     { COAP_OPT_URI_PATH, true, 0, 255, "Uri-Path" },
     { COAP_OPT_URI_QUERY, true, 0, 255, "Uri-Query" },
     { COAP_OPT_ACCEPT, false, 0, 2, "Accept" },
     { COAP_OPT_PROXY_URI, false, 1, 1034, "Proxy-Uri" },
     { COAP_OPT_PROXY_SCHEME, false, 1, 255, "Proxy-Scheme" },
};

#define nr_of_options ( sizeof(options) / sizeof(options[0]) )

static int compar(const void *p1, const void *p2)
{
     struct option *opt1 = (struct option *)p1;
     struct option *opt2 = (struct option *)p2;

     if (opt1->num < opt2->num)
	  return -1;
     if (opt1->num > opt2->num)
	  return 1;
     return 0;
}

static struct request *request_alloc() {
     struct request *req;
     
     /* Note: exiting the program upon memory allocation failure is acceptable only during prototyping */
     req = (struct request *)emalloc(sizeof(struct request));

     return req;
}

static void request_free()
{
     return;
}

static void request_init(struct request* req)
{
     TAILQ_INIT(&req->path);
}

#if 0
static int request_path_add(struct request *req, size_t len, uint8_t *str)
{
     struct uri_path *uri_path;
     
     uri_path = (struct uri_path *)emalloc(sizeof(struct uri_path));
     uri_path->len = len;
     uri_path->str = str;
     TAILQ_INSERT_TAIL(&req->path, uri_path, paths);
     return 0;
}
#endif

int parseopt(uint8_t *opt, size_t _left,
		uint16_t *delta, uint16_t *optlen)
{
     size_t left, hdrlen;

     hdrlen = 0;
     left = _left;

     *delta = HI_NIBBLE(*opt);
     *optlen = LO_NIBBLE(*opt);

     ++hdrlen;
     --left;

     switch(*delta) {
     case ONE_BYTE_DELTA:
	  if (left < 2)
	       return -1;
	  *delta = *(opt + 1) + 13;
	  ++hdrlen;
	  --left;
	  break;
     case TWO_BYTE_DELTA:
	  if (left < 3)
	       return -1;
	  *delta = ntohs(*(uint16_t *)(opt + 1)) + 269;
	  hdrlen += 2;
	  left -= 2;
	  break;
     case PAYLOAD_MKR:
	  if (*optlen != PAYLOAD_MKR)
	       return -1;
	  else
	       return 0;
     default:
	  break;
     };

     switch(*optlen) {
     case ONE_BYTE_OPTLEN:
	  if (left < 2)
	       return -1;
	  *optlen = *(opt + hdrlen + 1) + 13;
	  ++hdrlen;
	  --left;
	  break;
     case TWO_BYTE_OPTLEN:
	  if (left < 3)
	       return -1;
	  *optlen = ntohs(*(uint16_t *)(opt + hdrlen + 1)) + 269;
	  hdrlen += 2;
	  left -= 2;
	  break;
     case RESV_OPTLEN:
	  return -1;
     };

     if (left < *optlen)
	  return -1;

     return hdrlen;
}

int coap_req_get(uint16_t mid, uint8_t *buf, size_t buflen)
{
     struct request *req;
     uint8_t *opt, *optval;
     size_t offset, left;
     uint16_t delta, optlen, optnum;
     int hdrlen;
     struct option key, *prev = NULL;
     char str[256]; /* terminating null byte */

     req = request_alloc();
     if (req == NULL)
	  return -1;
     request_init(req);

     opt = buf;
     offset = 0;
     left = buflen;

     while (offset < buflen) {

	  hdrlen = parseopt(opt, left, &delta, &optlen);
	  if (hdrlen < 0) {
	       request_free();
	       return -1;
	  }
	  
	  offset += hdrlen;
	  left -= hdrlen;

	  if (delta == PAYLOAD_MKR) {
	       goto payload;
	  }

	  if (delta == 0 && prev != NULL && prev->repeat == false) {
	       printf("repeated %s\n", prev->name);
	       return -1;
	  }

	  optnum = (prev == NULL ? 0 : prev->num) + delta;
	  key.num = optnum;
	  prev = bsearch(&key, options, nr_of_options,
			sizeof(struct option), compar);
	  if (prev == NULL) {
	       printf("'%d': invalid option\n", key.num);
	       return -1;
	  }
	  else
	       printf("%s: option #%d\n", prev->name, prev->num);

	  optval = opt + hdrlen;

	  switch (optnum) {
	  case COAP_OPT_URI_PATH:
	       strncpy(str, (char *)optval, optlen);
	       str[optlen] = '\0';
	       printf("%s: str %s\n", prev->name, str);
	       break;
	  default:
	       break;
	  }

	  offset += optlen;
	  left -= optlen;
	  opt += offset;
     }

payload:
     ;
     
     return 0;
}
