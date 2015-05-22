#include "../defs.h"
#include "eprintf.h"

struct myentry {
     uint16_t mid;
     LIST_ENTRY(myentry) entries;
};

#define NBUCKETS	251

struct {
     LIST_HEAD(listhead, myentry) head;
} buckets[NBUCKETS];

struct myentry *lookup(uint16_t key, bool create)
{
     int h, *p;
     struct myentry *ep;

     h = key % NBUCKETS;
     for (ep = buckets[h].head.lh_first; ep != NULL; ep = ep->entries.le_next)
	  if (ep->mid == key)
	       return ep;
     if (create == true) {
	  ep = (struct myentry *)emalloc(sizeof(struct myentry));
	  LIST_INSERT_HEAD(&buckets[h].head, ep, entries);
	  return ep;
     }

     return ep;
}

void lremove(uint16_t key)
{
     struct myentry *ep;
     
     ep = lookup(key, false);
     if (ep != NULL) {
	  LIST_REMOVE(ep, entries);
	  free(ep);
     }
}

int main(int argc, char *argv[])
{
     int i;
     uint16_t mid;
     struct myentry *ep;

     for (i = 0; i < NBUCKETS; i++) {
	  LIST_INIT(&buckets[i].head);
     }
     
     srand(time(NULL));
     
     mid = rand() % 0xffff;

     /* Lookup entry */
     ep = lookup(mid, false);
     if (ep == NULL)
	  printf("mid %d not found\n", mid);
     else
	  printf("mid %d found\n", mid);

     /* Add entry */
     ep = lookup(mid, true);
     if (ep == NULL)
	  printf("mid %d not found\n", mid);
     else
	  printf("mid %d found\n", mid);

     /* Remove entry */
     lremove(mid);

     /* Lookup entry */
     ep = lookup(mid, false);
     if (ep == NULL)
	  printf("mid %d not found\n", mid);
     else
	  printf("mid %d found\n", mid);

     exit(EXIT_SUCCESS);
}
