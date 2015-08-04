#include <assert.h>
#include <stdint.h>		/* uintN_t */
#include <stdio.h>		/* printf() */
#include <stdlib.h>		/* malloc(), free() */
#include <string.h>		/* size_t */

#include "message.h"
#include "request.h"
#include "server.h"

/* Sorted array of messages for binary search */
static struct coap_code
{
  uint8_t val;
  char *name;
  int (*handler) (struct message *msg);
} coap_codes[] =
{
  {
  COAP_EMPTY, "Empty"},
  {
  COAP_REQ_GET, "GET", coap_req_get},
  {
  COAP_REQ_POST, "POST"},
  {
  COAP_REQ_PUT, "PUT"},
  {
  COAP_REQ_DEL, "DELETE"},
  {
  COAP_RESP_CREATED, "Created"},
  {
  COAP_RESP_DELETED, "Deleted"},
  {
  COAP_RESP_VALID, "Valid"},
  {
  COAP_RESP_CHANGED, "Changed"},
  {
  COAP_RESP_CONTENT, "Content"},
  {
  COAP_RESP_BAD_REQUEST, "Bad Request"},
  {
  COAP_RESP_UNAUTHORIZED, "Unauthorized"},
  {
  COAP_RESP_BAD_OPTION, "Bad Option"},
  {
  COAP_RESP_FORBIDDEN, "Forbidden"},
  {
  COAP_RESP_NOT_FOUND, "Not Found"},
  {
  COAP_RESP_METHOD_NOT_ALLOWED, "Method Not Allowed"},
  {
  COAP_RESP_NOT_ACCEPTABLE, "Not Acceptable"},
  {
  COAP_RESP_PRECONDITION_FAILED, "Precondition Failed"},
  {
  COAP_RESP_REQUEST_ENTITY_TOO_LARGE, "Request Entity Too Large"},
  {
  COAP_RESP_UNSUPPORTED_CONTENT_FORMAT, "Unsupported Content-Format"},
  {
  COAP_RESP_INTERNAL_SERVER_ERROR, "Internal Server Error"},
  {
  COAP_RESP_NOT_IMPLEMENTED, "Not Implemented"},
  {
  COAP_RESP_BAD_GATEWAY, "Bad Gateway"},
  {
  COAP_RESP_SERVICE_UNAVAILABLE, "Service Unavailable"},
  {
  COAP_RESP_GATEWAY_TIMEOUT, "Gateway Timeout"},
  {
  COAP_RESP_PROXYING_NOT_SUPPORTED, "Proxying Not Supported"}
};

#define nr_of_codes (sizeof(coap_codes) / sizeof(coap_codes[0]))

static int
compare (const void *p1, const void *p2)
{
  struct coap_code *code1 = (struct coap_code *) p1;
  struct coap_code *code2 = (struct coap_code *) p2;

  if (code1->val < code2->val)
    return -1;
  if (code1->val > code2->val)
    return 1;
  return 0;
}

#define COAP_HEADER_VERSION		0x01

static inline uint8_t
coap_header_ver (uint8_t c)
{
  return (c >> 6);
}

static inline uint8_t
coap_header_type (uint8_t c)
{
  return ((c >> 4) & 0x3);
}

#define COAP_MAX_TKLEN		8

static inline uint8_t
coap_header_tklen (uint8_t c)
{
  return (c & 0xf);
}

char *type_str[] = { "CON", "NON", "ACK", "RST" };

static inline const char *
type2str (uint8_t type)
{
     return type_str[type];
}

int
coap_header_parse (struct message *msg)
{
  uint8_t *hdr;
  struct coap_code key, *code;

  hdr = msg->data;

  if (coap_header_ver (*hdr) != COAP_HEADER_VERSION)
    {
      warning ("invalid version num");
      return -1;
    }

  msg->type = coap_header_type (*hdr);

  msg->tklen = coap_header_tklen (*hdr);
  if (msg->tklen > COAP_MAX_TKLEN)
    {
      warning ("tklen > 8");
      return -1;
    }

  /* Low-cost sanity check */
  /* Less visible than in calling function but saves the code lookup time below */
  if (msg->len < COAP_HDR_SIZE + msg->tklen)
    {
      warning ("truncated msg");
      return -1;
    }

  key.val = *(hdr + 1);
  code = bsearch (&key, coap_codes, nr_of_codes,
		  sizeof (struct coap_code), compare);

  if (code == NULL)
    {
      warning ("unknown code");
      return -1;
    }

  msg->code = code->val;

  msg->id = htons (*(uint16_t *) (hdr + 2));

  system_log ("%s:  type: %s  tklen: %d  code: %s  mid: %d\n",
  	      __func__, type2str (msg->type), msg->tklen, code->name, msg->id);

  return 0;
}

struct message *
coap_message_new ()
{
  struct message *msg;

  msg = xmalloc (sizeof (struct message));

  return msg;
}

void
coap_message_free (struct message *msg)
{
  assert(msg != NULL);

  if (msg != NULL)
    free (msg);
}

int
coap_message_add (struct message *msg)
{
  return -1;
}

int
coap_message_find (struct message *msg)
{
  return -1;
}

int
coap_message_del (struct message *msg)
{
  return -1;
}

# if 1 
void *
coap_message_process (void *arg)
{
  struct message *msg = (struct message *)arg;
  uint8_t val;
  int ret;

  val = msg->code;

  if (coap_codes[val].handler != NULL) {
    ret = coap_codes[val].handler(msg);
  }

  if (ret == 0)
    return NULL;
  
  return NULL;
}
#else
void *
coap_message_process (void *arg)
{
  struct message *msg = (struct message *)arg;
  if (msg->len > COAP_HDR_SIZE + msg->tklen) {
    /* Options are present */
    ;
  }
  else {
    if (msg->type == COAP_TYPE_CON && msg->code == COAP_EMPTY) {
      /* "CoAP ping" */
      ;
    }
    else {
      /* Something else */
      ;
    }
  }
  return NULL;
}
#endif
