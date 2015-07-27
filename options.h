#include <stdbool.h>
#include <stdint.h>	/* uintN_t */
#include <stdlib.h>	/* size_t */

/* CoAP Option Numbers */
enum {
     COAP_OPT_RESV0 = 0x00,
     COAP_OPT_IF_MATCH = 0x01,
     COAP_OPT_URI_HOST = 0x03,
     COAP_OPT_ETAG = 0x04,
     COAP_OPT_IF_NONE_MATCH = 0x05,
     COAP_OPT_URI_PORT = 0x07,
     COAP_OPT_LOCATION_PATH = 0x08,
     COAP_OPT_URI_PATH = 0x0B,
     COAP_OPT_CONTENT_FORMAT = 0x0C,
     COAP_OPT_MAX_AGE = 0x0D,
     COAP_OPT_URI_QUERY = 0x0F,
     COAP_OPT_ACCEPT = 0x11,
     COAP_OPT_LOCATION_QUERY = 0x14,
     COAP_OPT_PROXY_URI = 0x23,
     COAP_OPT_PROXY_SCHEME = 0x27,
     COAP_OPT_SIZE1 = 0x3C,
     COAP_OPT_RESV1 = 0x80,
     COAP_OPT_RESV2 = 0x84,
     COAP_OPT_RESV3 = 0x88,
     COAP_OPT_RESV4 = 0x8C
};

struct option {
     uint16_t num;
     bool repeat;
     uint16_t minlen;
     uint16_t maxlen;
     char *name;
};

enum {
     ONE_BYTE_DELTA = 0xD,
     TWO_BYTE_DELTA,
     PAYLOAD_MKR
};

enum {
     ONE_BYTE_OPTLEN = 0xD,
     TWO_BYTE_OPTLEN,
     RESV_OPTLEN
};

#define HI_NIBBLE(b) (((b) >> 4) & 0x0f)
#define LO_NIBBLE(b) ((b) & 0x0f)
