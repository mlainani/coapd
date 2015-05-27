#include <search.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "sys/queue.h"

#include "eprintf.h"

enum { NHASH = 251 };

struct msg {
     uint16_t id;
     struct sockaddr_in6 src;
     LIST_ENTRY(msg) entries;
};

extern void msgtab_init();

extern struct msg *msgtab_lookup(uint16_t id, struct sockaddr_in6 *src, bool create);

extern void msgtab_remove(uint16_t id);

extern void msgtab_walk();
