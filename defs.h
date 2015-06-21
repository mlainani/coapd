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

#define COAP_DEFAULT_PORT		5683

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
