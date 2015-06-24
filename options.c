#include <stdio.h>
#include <arpa/inet.h>

#include "options.h"

struct option options[] = {
     { COAP_OPT_IF_MATCH, true, 0, 8, "If-Match" },
     { COAP_OPT_URI_HOST, false, 1, 255, "Uri-Host" },
     { COAP_OPT_ETAG, true, 1, 8, "ETag" },
     { COAP_OPT_IF_NONE_MATCH, false, 0, 0, "If-None-Match" },
     { COAP_OPT_URI_PORT, false, 0, 2, "Uri-Port" },
     { COAP_OPT_LOCATION_PATH, true, 0, 255, "Location-Path" },
     { COAP_OPT_URI_PATH, true, 0, 255, "Uri-Path" },
     { COAP_OPT_CONTENT_FORMAT, false, 0, 2, "Content-Format" },
     { COAP_OPT_MAX_AGE, false, 0, 4, "Max-Age" },
     { COAP_OPT_URI_QUERY, true, 0, 255, "Uri-Query" },
     { COAP_OPT_ACCEPT, false, 0, 2, "Accept" },
     { COAP_OPT_LOCATION_QUERY, true, 0, 255, "Location-Query" },
     { COAP_OPT_PROXY_URI, false, 1, 1034, "Proxy-Uri" },
     { COAP_OPT_PROXY_SCHEME, false, 1, 255, "Proxy-Scheme" },
     { COAP_OPT_SIZE1, false, 0, 4, "Size1" }
};

static int option_delta(uint8_t *opt, size_t left, uint16_t *delta)
{
     int ret = -1;

     *delta = *opt >> 0x4;
 
     switch(*delta) {
     case ONE_BYTE_DELTA:
	  if (left > 1) {
	       *delta = *(opt + 1) + 13;
	       ret = 0;
	  }
	  break;
     case TWO_BYTE_DELTA:
	  if (left > 2) {
	       *delta = ntohs(*(uint16_t *)(opt + 1)) + 269;
	       ret = 0;
	  }
	  break;
     case PAYLOAD_MKR:
	  if ((*opt & 0xf) == 0xf)
	       ret = 0;
	  break;
     default:
	  ret = 0;
     };

     return ret;
}

static inline int option_len(uint8_t *opt, size_t left, uint16_t *optlen)
{
     int ret = -1;

     *optlen = *opt >> 0x4;
 
     switch(*optlen) {
     case ONE_BYTE_OPTLEN:
	  if (left > 1) {
	       *optlen = *(opt + 1) + 13;
	       ret = 0;
	  }
	  break;
     case TWO_BYTE_OPTLEN:
	  if (left > 2) {
	       *optlen = ntohs(*(uint16_t *)(opt + 1)) + 269;
	       ret = 0;
	  }
	  break;
     case RESV_OPTLEN:
	  if ((*opt & 0xf) == 0xf)
	       ret = 0;
	  break;
     default:
	  ret = 0;
     };

     return ret;
}

static int process(uint8_t *opt, size_t left,
		   uint16_t *delta, uint16_t *optlen,
		   size_t *skiplen)
{
     int ret = -1;
     return ret;
}

#if 1
int parse_options(uint8_t *buf, size_t buflen, uint8_t codeval)
{
     uint16_t delta, optlen, num;
     uint8_t *curr;
     size_t skiplen, offset = 0;

     curr = buf;
     while (offset < buflen) {
	  process(curr, buflen - offset, &delta, &optlen, &skiplen);
	  /* Move curr and increase offset */
      }
}
#else
int parse_options(uint8_t *buf, size_t buflen, uint8_t codeval)
{
     uint16_t prev, optnum, delta;
     uint8_t *opt, *optval, *curr;
     size_t len, offset, optlen;

     prev = 0;
     opt = buf;
     offset = len = 0;
     
     curr = buf;
     while (curr < buf + buflen) {

	  delta = *opt >> 0x4;
	  optlen = *opt & 0xf;
	  
	  switch(delta) {

	  case ONE_BYTE_DELTA:
	       if (buflen - offset > 1)
		    delta = *(opt + 1) + 13;
	       else
		    printf("message format error\n");
	       break;

	  case TWO_BYTE_DELTA:
	       if (buflen - offset > 2)
		    delta = ntohs(*(uint16_t *)(opt + 1)) + 269;
	       else
		    printf("message format error\n");
	       break;

	  case PAYLOAD_MKR_NIBBLE:
	       if ((*opt & 0xf) == 0xf)
		    printf("finished parsing options\n");
	       else
		    printf("message format error\n");

	  default:
	       ret = 0;
	  };
	       


	  optnum = prev + option_delta(opt, buflen - offset, &delta);
	  optlen = option_len(opt);

	  /* Look for the option number */
	  /* Update len: several places */
	  len += 0;
	  /* Add the number of parsed bytes  */
	  offset += len;
     }

     return 0;
}
#endif
