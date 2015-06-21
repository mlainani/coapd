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

int parse_options(uint8_t *buf, size_t len, uint8_t codeval)
{
     return 0;
}
