#include "msgtab.h"

static struct {
     LIST_HEAD(listhead, msg) head;
} msgtab[NHASH];

void msgtab_init()
{
     int i;

     for (i = 0; i < NHASH; i++) {
	  LIST_INIT(&msgtab[i].head);
     }
}

struct msg *msgtab_lookup(uint16_t id, struct sockaddr_in6 *src, bool create)
{
     int h;
     struct msg *msgp;

     h = id % NHASH;
     for (msgp = msgtab[h].head.lh_first; msgp != NULL; msgp = msgp->entries.le_next)
	  if (msgp->id == id)
	       return msgp;
     if (create == true) {
	  msgp = (struct msg *)emalloc(sizeof(struct msg));
	  msgp->id = id;
	  if (src != NULL)
	       memcpy(&msgp->src, src, sizeof(msgp->src));
	  LIST_INSERT_HEAD(&msgtab[h].head, msgp, entries);
	  return msgp;
     }

     return msgp;
}

void msgtab_remove(uint16_t id)
{
     struct msg *msgp;
     
     msgp = msgtab_lookup(id, NULL, false);
     if (msgp != NULL) {
	  LIST_REMOVE(msgp, entries);
	  free(msgp);
     }
}

void msgtab_walk()
{
     int i;
     struct msg *msgp;
     char str[INET_ADDRSTRLEN];

     for (i = 0; i < NHASH; i++) {
	  for (msgp = msgtab[i].head.lh_first; msgp != NULL;
	       msgp = msgp->entries.le_next) {
	       inet_ntop(AF_INET6, &msgp->src.sin6_addr,str, sizeof(str));
	       printf("mid: %d\tsrc: %s\n", msgp->id, str);
	  }
     }
}
