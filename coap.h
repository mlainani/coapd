#include <signal.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#include <sys/socket.h>
#include <sys/time.h>

#define COAP_MSG_MAX_SIZE	1152
#define COAP_MSG_MAX_PAYLOAD	1024

/* #define COAP_HDR_VER(a)		((a >> 6) & 0x3) */
#define COAP_HDR_TYPE(a)	((a >> 4) & 0x3)
#define COAP_HDR_TKLEN(a)	(a & 0xff)

#define COAP_HDR_CODE_CLASS(a)	((a >> 1) & 0x7)
#define COAP_HDR_CODE_DETAIL(a)	(a && 0x1f)

#define COAP_OPT_DELTA(a)	((a >> 4) & 0xf)
#define COAP_OPT_LEN(a)		(a & 0xf)

#define COAP_VERSION		0x1

#define COAP_HDR_SIZE		4

#define COAP_MSG_TYPE_CON	0x0
#define COAP_MSG_TYPE_NON	0x1
#define COAP_MSG_TYPE_ACK	0x2
#define COAP_MSG_TYPE_RST	0x3

#define COAP_MSG_PAYLOAD_MKR	0xFF

struct coap_msg {
     unsigned int version:2;
     unsigned int type:2;
     unsigned int tklen:4;
     unsigned int code:8;
     unsigned int id:16;
     unsigned char body[0];
};

/* Transmission parameters */

#define COAP_ACK_TIMEOUT	2		/* seconds */
#define COAP_ACK_RANDOM_FACTOR	1.5F
#define COAP_MAX_RETRANSMIT	4
#define COAP_NSTART		1
#define COAP_DEFAULT_LEISURE	5		/* seconds */
#define COAP_PROBING_RATE	1		/* byte/second */

#define MAX_TRANSMIT_SPAN	(COAP_ACK_TIMEOUT * ((2 ^ COAP_MAX_RETRANSMIT) - 1) \ * COAP_ACK_RANDOM_FACTOR)

#define COAP_MAX_TRANSMIT_SPAN	45	/* seconds */
#define COAP_MAX_TRANSMIT_WAIT	93	/* seconds */
#define COAP_MAX_LATENCY	100	/* seconds */
#define COAP_PROCESSING_DELAY	2	/* seconds */
#define COAP_MAX_RTT		202	/* seconds */
#define COAP_EXCHANGE_LIFETIME	247	/* seconds */
#define COAP_NON_LIFETIME	145	/* seconds */

/* CoAP Method Codes */

enum {
     COAP_CODE_GET = 0x01,	/* 0.01 */
     COAP_CODE_POST,	/* 0.02 */
     COAP_CODE_PUT,	/* 0.03 */
     COAP_CODE_DEL		/* 0.04 */
};

/* CoAP Response Codes */

enum {
     COAP_RESP_CREATED = 0x41,			/* 2.01 */
     COAP_RESP_DELETED,				/* 2.02 */
     COAP_RESP_VALID,				/* 2.03 */
     COAP_RESP_CHANGED,				/* 2.04 */
     COAP_RESP_CONTENT,				/* 2.05 */

     COAP_RESP_BAD_REQUEST = 0x80,			/* 4.00 */
     COAP_RESP_UNAUTHORIZED,			/* 4.01 */
     COAP_RESP_BAD_OPTION,				/* 4.02 */
     COAP_RESP_FORBIDDEN,				/* 4.03 */
     COAP_RESP_NOT_FOUND,				/* 4.04 */
     COAP_RESP_METHOD_NOT_ALLOWED,			/* 4.05 */
     COAP_RESP_NOT_ACCEPTABLE,			/* 4.06 */
     COAP_RESP_PRECONDITION_FAILED = 0x8c,		/* 4.12 */
     COAP_RESP_REQUEST_ENTITY_TOO_LARGE,		/* 4.13 */
     COAP_RESP_UNSUPPORTED_CONTENT_FORMAT = 0x8f,	/* 4.15 */

     COAP_RESP_INTERNAL_SERVER_ERROR = 0xa0,	/* 5.00 */
     COAP_RESP_NOT_IMPLEMENTED,			/* 5.01 */
     COAP_RESP_BAD_GATEWAY,			/* 5.02 */
     COAP_RESP_SERVICE_UNAVAILABLE,		/* 5.03 */
     COAP_RESP_GATEWAY_TIMEOUT,			/* 5.04 */
     COAP_RESP_PROXYING_NOT_SUPPORTED		/* 5.05 */
};

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

#define COAP_DEFAULT_PORT		5683

typedef struct {
     uint16_t num;
     bool repeat;
     uint16_t minlen;
     uint16_t maxlen;
     char name[];
} coap_opt_t;

coap_opt_t options[] = {
     { COAP_OPT_IF_MATCH, true, 0, 8, "If-Match" },
     { COAP_OPT_URI_HOST, false, 1, 255, "Uri-Host" }
};

