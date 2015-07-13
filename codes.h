#include <stdint.h>	/* uintN_t */

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
