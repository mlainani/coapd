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

int parse_options(uint8_t *buf, size_t buflen, uint8_t codeval)
{
     uint8_t *opt/* , *optval */;
     size_t offset, left;
     uint16_t delta, optlen;
     struct option key, *prev = NULL;

     opt = buf;
     offset = 0;
     left = buflen;

     while (offset < buflen) {
	  delta = HI_NIBBLE(*opt);
	  optlen = LO_NIBBLE(*opt);
	  
	  offset++;
	  left--;

	  switch(delta) {
	  case ONE_BYTE_DELTA:
	       if (left < 2)
		    return -1;
	       delta = *(opt + 1) + 13;
	       offset++;
	       left--;
	       break;
	  case TWO_BYTE_DELTA:
	       if (left < 3)
		    return -1;
	       delta = ntohs(*(uint16_t *)(opt + 1)) + 269;
	       offset += 2;
	       left -= 2;
	       break;
	  case PAYLOAD_MKR:
	       if (optlen != PAYLOAD_MKR)
		    return -1;
	       else {
		    /* TODO: make sure to point to the beginning of the payload; */
		    goto payload;
	       }
	  };

	  switch(optlen) {
	  case ONE_BYTE_OPTLEN:
	       if (left < 2)
		    return -1;
	       optlen = *(opt + offset + 1) + 13;
	       offset++;
	       left--;
	       break;
	  case TWO_BYTE_OPTLEN:
	       if (left < 3)
		    return -1;
	       optlen = ntohs(*(uint16_t *)(opt + offset + 1)) + 269;
	       offset += 2;
	       left -= 2;
	       break;
	  case RESV_OPTLEN:
	       return -1;
	  };

	  if (delta == 0 && prev != NULL && prev->repeat == false) {
	       printf("repeated %s\n", prev->name);
	       return -1;
	  }

	  key.num = (prev == NULL ? 0 : prev->num) + delta;
	  prev = bsearch(&key, options, nr_of_options,
			sizeof(struct option), compar);
	  if (prev == NULL) {
	       printf("'%d': unknown option\n", key.num);
	       return -1;
	  }
	  else
	       printf("%s: option #%d\n", prev->name, prev->num);

	  offset += optlen;
	  opt += offset;
     }

payload:
     ;
     
     return 0;
}
