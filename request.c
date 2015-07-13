#include "request.h"

#include "eprintf.h"

static struct option options[] = {
     { COAP_OPT_IF_MATCH, true, 0, 8, "If-Match" },
     { COAP_OPT_URI_HOST, false, 1, 255, "Uri-Host" },
     { COAP_OPT_ETAG, true, 1, 8, "ETag" },
     { COAP_OPT_IF_NONE_MATCH, false, 0, 0, "If-None-Match" },
     { COAP_OPT_URI_PORT, false, 0, 2, "Uri-Port" },
     { COAP_OPT_URI_PATH, true, 0, 255, "Uri-Path" },
     { COAP_OPT_URI_QUERY, true, 0, 255, "Uri-Query" },
     { COAP_OPT_ACCEPT, false, 0, 2, "Accept" },
     { COAP_OPT_PROXY_URI, false, 1, 1034, "Proxy-Uri" },
     { COAP_OPT_PROXY_SCHEME, false, 1, 255, "Proxy-Scheme" },
};

#define nr_of_options ( sizeof(options) / sizeof(options[0]) )

struct request *request_alloc() {
     struct request *req;
     
     /* Note: exiting the program upon memory allocation failure is acceptable only during prototyping */
     req = (struct request *)emalloc(sizeof(struct request));

     return req;
}

void request_init(struct request* req)
{
     TAILQ_INIT(&req->path);
}

int request_path_add(struct request *req, size_t len, uint8_t *str)
{
     struct uri_path *uri_path;
     
     uri_path = (struct uri_path *)emalloc(sizeof(struct uri_path));
     uri_path->len = len;
     uri_path->str = str;
     TAILQ_INSERT_TAIL(&req->path, uri_path, paths);
     return 0;
}
