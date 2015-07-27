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

#if 0
static struct request *request_alloc();
static void request_init();
static int request_path_add(struct request *req, size_t len, uint8_t *str);
static void request_free();
#endif

extern int coap_req_get(uint16_t mid, uint8_t *buf, size_t buflen);
