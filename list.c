#include "defs.h"

/*
		PDP-7
		
		Unix 1-4
			
	Unix 5-6		PWB

BSD 1-2			Unix 7
BSD 3-4.1	Unix 8	Unix/32V	Xenix 1-2.3
*/

/* Unix 7	Unix 8	Unix/32V	Xenix 1-2.3 */

char *os_names[] = {
     "Unnamed PDP-7 operating system",
     "Unix Version 1 to 4",
     "Unix Version 5 to 6",
     "PWB/Unix",
     "BSD 1.0 to 2.0",
     "Unix Version 7",
     "Unix Version 8",
     "Unix/32V",
     "Xenix 1.0 to 2.3",
     "BSD 3.0 to 4.1",
};

struct entry {
     char *name;
     TAILQ_ENTRY(entry) entries;
};

TAILQ_HEAD(tailhead, entry);

struct vertex {
     char *name;
     struct tailhead head;
} vertices[] = {
     { "Unnamed PDP-7 operating system" },
     { "Unix Version 1 to 4" },
     { "Unix Version 5 to 6" },
     { "PWB/Unix" },
     { "BSD 1.0 to 2.0" },
     { "Unix Version 7" },
     { "Unix Version 8" },
     { "Unix/32V" },
     { "Xenix 1.0 to 2.3" },
     { "BSD 3.0 to 4.1" },
};

#define nr_of_vertices (sizeof(vertices) / sizeof(vertices[0]))

int main()
{
     int i;
     struct tailhead *head;
     struct entry *v;

     for (i = 0; i < nr_of_vertices; i++) {
	  printf("%d %s\n", i, vertices[i].name);
	  TAILQ_INIT(&vertices[i].head);
     }

     v = malloc(sizeof(struct entry));
     v->name = "Unix Version 8";
     TAILQ_INSERT_TAIL(&vertices[5].head, v, entries);
     
     v = malloc(sizeof(struct entry));
     v->name = "Unix/32V";
     TAILQ_INSERT_TAIL(&vertices[5].head, v, entries);
     
     v = malloc(sizeof(struct entry));
     v->name = "Xenix 1.0 to 2.3";
     TAILQ_INSERT_TAIL(&vertices[5].head, v, entries);
     
     head = &vertices[5].head;

     if (&vertices[5].head.tqh_first != NULL) {
	  v = head->tqh_first;
	  printf("peekaboo!\n");
     }
     
     for (v = vertices[5].head.tqh_first; v != NULL; v = v->entries.tqe_next)
      	  printf("%s\n", v->name);

     /* 
      * TAILQ_HEAD(tailhead, entry) head;
      * /\* struct tailhead *headp;		/\\* List head. *\\/ *\/
      * struct entry *n1, *n2, *n3, *np;
      * 
      * TAILQ_INIT(&head);			/\* Initialize the list. *\/
      * 
      * n1 = malloc(sizeof(struct entry));
      * n1->name = "Unix Version 8";
      * TAILQ_INSERT_TAIL(&head, n1, entries);
      * 
      * n2 = malloc(sizeof(struct entry));
      * n2->name = "Unix/32V";
      * TAILQ_INSERT_TAIL(&head, n2, entries);
      * 
      * n3 = malloc(sizeof(struct entry));
      * n3->name = "Xenix 1.0 to 2.3";
      * TAILQ_INSERT_TAIL(&head, n3, entries);
      * 
      * for (np = head.tqh_first; np != NULL; np = np->entries.tqe_next)
      * 	  printf("%s\n", np->name);
      */

     exit(0);
}
