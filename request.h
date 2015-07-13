#include <stdint.h>	/* uintN_t */
#include <stdlib.h>	/* size_t */
#include <sys/queue.h>	/* tail queues */

#include "options.h"

struct uri_path {
     size_t len;
     uint8_t *str;
     TAILQ_ENTRY(uri_path) paths;
};

struct request {
     TAILQ_HEAD(tailhead, uri_path) path;
};

extern struct request *request_alloc();
extern void request_init();
extern int request_path_add(struct request *req, size_t len, uint8_t *str);
extern void request_free();
extern int coap_req_get(uint16_t mid, uint8_t *buf, size_t buflen);
