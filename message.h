#ifndef MESSAGE_H
#define MESSAGE_H

#include <netinet/in.h>		/* struct in6_addr */
#include <stdint.h>		/* uintN_t */
#include <sys/queue.h>

/* Fixed-size header length */
#define COAP_HDR_SIZE	4

/* RFC 7252 - Section 4.6 */
#define COAP_MSG_MAX_SIZE	1152

/* CoAP types */

enum
{
  COAP_TYPE_CON = 0x0,
  COAP_TYPE_NON,
  COAP_TYPE_ACK,
  COAP_TYPE_RST
};

/* CoAP codes */
enum
{
  COAP_EMPTY = 0x0,		/* 0.00 */

  COAP_REQ_GET = 0x01,		/* 0.01 */
  COAP_REQ_POST,		/* 0.02 */
  COAP_REQ_PUT,			/* 0.03 */
  COAP_REQ_DEL,			/* 0.04 */

  COAP_RESP_CREATED = 0x41,	/* 2.01 */
  COAP_RESP_DELETED,		/* 2.02 */
  COAP_RESP_VALID,		/* 2.03 */
  COAP_RESP_CHANGED,		/* 2.04 */
  COAP_RESP_CONTENT,		/* 2.05 */

  COAP_RESP_BAD_REQUEST = 0x80,	/* 4.00 */
  COAP_RESP_UNAUTHORIZED,	/* 4.01 */
  COAP_RESP_BAD_OPTION,		/* 4.02 */
  COAP_RESP_FORBIDDEN,		/* 4.03 */
  COAP_RESP_NOT_FOUND,		/* 4.04 */
  COAP_RESP_METHOD_NOT_ALLOWED,	/* 4.05 */
  COAP_RESP_NOT_ACCEPTABLE,	/* 4.06 */
  COAP_RESP_PRECONDITION_FAILED = 0x8c,	/* 4.12 */
  COAP_RESP_REQUEST_ENTITY_TOO_LARGE,	/* 4.13 */
  COAP_RESP_UNSUPPORTED_CONTENT_FORMAT = 0x8f,	/* 4.15 */

  COAP_RESP_INTERNAL_SERVER_ERROR = 0xa0,	/* 5.00 */
  COAP_RESP_NOT_IMPLEMENTED,	/* 5.01 */
  COAP_RESP_BAD_GATEWAY,	/* 5.02 */
  COAP_RESP_SERVICE_UNAVAILABLE,	/* 5.03 */
  COAP_RESP_GATEWAY_TIMEOUT,	/* 5.04 */
  COAP_RESP_PROXYING_NOT_SUPPORTED	/* 5.05 */
};

struct request
{
  TAILQ_HEAD (tailhead, uri_path) path;
};

struct response
{
};

struct message
{
  size_t len;
  struct in6_addr src;
  uint8_t type;
  uint8_t tklen;
  uint8_t code;
  uint16_t id;
  uint8_t *token;
  uint8_t *opt;
  uint8_t *payload;
  uint8_t data[COAP_MSG_MAX_SIZE];
  LIST_ENTRY (message) entries;
  union
  {
    struct request req;
    struct response resp;
  };
};

extern int coap_header_parse (struct message *msg);

extern struct message *coap_message_new ();

extern void coap_message_free (struct message *msg);

extern void *coap_message_process (void *arg);

#endif /* MESSAGE_H */
