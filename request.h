#include <sys/queue.h>		/* tail queues */

#include "options.h"
#include "message.h"

struct uri_path
{
  size_t len;
  uint8_t *str;
    TAILQ_ENTRY (uri_path) paths;
};

#if 0
struct request
{
  TAILQ_HEAD (tailhead, uri_path) path;
};

static struct request *request_alloc ();
static void request_init ();
static int request_path_add (struct request *req, size_t len, uint8_t * str);
static void request_free ();
#endif

extern int coap_req_get (struct message *msg);
