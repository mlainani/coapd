#define _GNU_SOURCE
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#include <sys/socket.h>
#include <sys/time.h>

#if 1
#define handle_error(msg)	\
     do { perror(msg); } while (0)
#else
#define handle_error(msg)	\
     do { perror(msg); exit(EXIT_FAILURE); } while (0)
#endif

/* RFC 7252 - Section 4.6 */
#define COAP_MSG_MAX_SIZE	1152
#define COAP_MSG_MAX_PAYLOAD	1024

#define COAP_HDR_VER(a)		((a >> 6) & 0x3)
#define COAP_HDR_TYPE(a)	((a >> 4) & 0x3)
#define COAP_HDR_TKLEN(a)	(a & 0xff)

#define COAP_HDR_CODE_CLASS(a)	((a >> 1) & 0x7)
#define COAP_HDR_CODE_DETAIL(a)	(a && 0x1f)

#define COAP_OPT_DELTA(a)	((a >> 4) & 0xf)
#define COAP_OPT_LEN(a)		(a & 0xf)

#define COAP_HDR_SIZE		4

#define COAP_VERSION		0x01

#define COAP_VERSION_MASK	0xc0
#define COAP_VERSION_BITS	0x40

#define COAP_TYPE_MASK		0x30

#define COAP_TYPE_CON_BITS	0x0
#define COAP_TYPE_NON_BITS	0x10
#define COAP_TYPE_ACK_BITS	0x20
#define COAP_TYPE_RST_BITS	0x30

#define COAP_MSG_PAYLOAD_MKR	0xFF


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


/* CoAP types */

enum {
     COAP_TYPE_CON = 0x0,
     COAP_TYPE_NON,
     COAP_TYPE_ACK,
     COAP_TYPE_RST
};

char *type_str[] = {"CON", "NON", "ACK", "RST"};

/* CoAP codes */

enum {
     COAP_EMPTY = 0x0,					/* 0.00 */
     
     COAP_REQ_GET = 0x01,				/* 0.01 */
     COAP_REQ_POST,					/* 0.02 */
     COAP_REQ_PUT,					/* 0.03 */
     COAP_REQ_DEL,					/* 0.04 */

     COAP_RESP_CREATED = 0x41,				/* 2.01 */
     COAP_RESP_DELETED,					/* 2.02 */
     COAP_RESP_VALID,					/* 2.03 */
     COAP_RESP_CHANGED,					/* 2.04 */
     COAP_RESP_CONTENT,					/* 2.05 */

     COAP_RESP_BAD_REQUEST = 0x80,			/* 4.00 */
     COAP_RESP_UNAUTHORIZED,				/* 4.01 */
     COAP_RESP_BAD_OPTION,				/* 4.02 */
     COAP_RESP_FORBIDDEN,				/* 4.03 */
     COAP_RESP_NOT_FOUND,				/* 4.04 */
     COAP_RESP_METHOD_NOT_ALLOWED,			/* 4.05 */
     COAP_RESP_NOT_ACCEPTABLE,				/* 4.06 */
     COAP_RESP_PRECONDITION_FAILED = 0x8c,		/* 4.12 */
     COAP_RESP_REQUEST_ENTITY_TOO_LARGE,		/* 4.13 */
     COAP_RESP_UNSUPPORTED_CONTENT_FORMAT = 0x8f,	/* 4.15 */

     COAP_RESP_INTERNAL_SERVER_ERROR = 0xa0,		/* 5.00 */
     COAP_RESP_NOT_IMPLEMENTED,				/* 5.01 */
     COAP_RESP_BAD_GATEWAY,				/* 5.02 */
     COAP_RESP_SERVICE_UNAVAILABLE,			/* 5.03 */
     COAP_RESP_GATEWAY_TIMEOUT,				/* 5.04 */
     COAP_RESP_PROXYING_NOT_SUPPORTED			/* 5.05 */
};

/* Sorted array of codes for binary search */
struct ci {
     uint8_t code;
     char *name;
} codes[] = {
     { COAP_EMPTY, "Empty" },

     { COAP_REQ_GET, "GET" },
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

#define nr_of_codes (sizeof(codes) / sizeof(codes[0]))

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
     char *name;
} option;

option options[] = {
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

#define NOPTS ( sizeof(options) / sizeof(options[0]) )

/* Max number of Uri-Path options in a message */
#define URI_PATH_VEC_MAX_SIZE	32

static inline uint8_t coap_hdr_ver(uint8_t c)
{
     return(c >> 6);
}

static inline uint8_t coap_hdr_type(uint8_t c)
{
     return((c >> 4) & 0x3);
}

static inline uint8_t coap_hdr_tklen(uint8_t c)
{
     return(c & 0xf);
}

struct server {
     int sockfd;
};
