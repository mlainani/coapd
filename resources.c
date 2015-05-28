/* 
 * Apparently, GCC doesn't allow variable size arrays to be nested in a
 * structure like so:
 * 
 * 
 * struct path {
 *      int (*get)();
 *      char *tokens[];
 * } paths[] = {
 *      { dummy_get, { "path", "to", "dummy" } },
 * };
 * 
 * compiler returns
 * 
 * error: initialization of flexible array member in a nested context
 *       { dummy_get, { "path", "to", "dummy" } },
 *       ^
 * 
 * To circumvent this obstacle, we can:
 * 
 * 1) Fix the array size and waste memory
 * 2) Process the path with strtok() as a single string at run time
 * 
 * struct resource {
 *      char *path;
 *      int (*get)();
 * } resources[] = {
 *      { "path/to/dummy", dummy_get },
 * };
 */

#include <search.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/queue.h>

#include "linux/limits.h"

#include "eprintf.h"

static void *root = NULL;

int some_get() {
     printf("Voila some!\n");
     return 0;
}

int other_get() {
     printf("Voila other!\n");
     return 0;
}

int short_get() {
     printf("Voila short!\n");
     return 0;
}

int long_get() {
     printf("Voila long!\n");
     return 0;
}

enum { PATH_MAX_TOKENS = 4 };

struct path {
     int (*get)();
     char *tokens[PATH_MAX_TOKENS];
} paths[] = {
     { some_get, { "path", "to", "some" } },
     { other_get, { "path", "to", "other" } },
     { short_get, { "short" } },
     { long_get, { "path", "to", "very", "long" } },
};

#define NPATHS ( sizeof(paths) / sizeof(paths[0]) )

int dummy_get() {
     printf("Voila dummy!\n");
     return 0;
}

struct path dummies[] = {
     { some_get, { "path", "to", "dummy" } },
     { other_get, { "path", "dummy", "other" } },
     { short_get, { "dummy" } },
     { long_get, { "path", "to", "very", "dummy" } },
};

#define NDUMMIES ( sizeof(dummies) / sizeof(dummies[0]) )

/* Linked list item == path graph vertex adjacency list element */
struct item {
     char *name;
     LIST_ENTRY(item) entries;
};

/* Binary tree node == path graph vertex */
struct node {
     char *name;
     int (*get)();
     LIST_HEAD(listhead, item) head;
};

static int compar(const void *p1, const void *p2)
{
     struct node *n1 = (struct node *)p1;
     struct node *n2 = (struct node *)p2;

     return strcmp(n1->name, n2->name);
}

void path_add(struct path *path)
{
     struct node *curr, *prev;
     int i;
     char *str;
     void *val;
     struct item *item;
     bool found;

     for (prev = NULL, i = 0, str = path->tokens[i];
	  i < PATH_MAX_TOKENS && str != NULL;
	  str = path->tokens[++i]) {

	  /* 
           * Allocate and initialize the B-tree node element. Needed even if we
	   * just want to search a node.
           */

	  curr = (struct node *)emalloc(sizeof(struct node));
	  curr->name = str;

	  /* 
           * Add the node to the B-tree if it doesn't exist yet. If it exists
	   * already, free the node allocated for the search operation and store
	   * its address for upcoming processing.
           */

	  val = tsearch((void *)curr, &root, compar);

	  if (val == NULL)
	       exit(EXIT_FAILURE);

	  if (*(struct node **)val == curr) {
	       curr->get = NULL;
	       LIST_INIT(&curr->head);
	  }
	  else {
	       free(curr);
	       curr = *(struct node **)val;
	  }

	  if (prev != NULL) {
	       found = false;
	       for (item = prev->head.lh_first; item != NULL;
		    item = item->entries.le_next) {
		    if (curr->name == item->name) {
			 found = true;
			 break;
		    }
	       }
	       if (found == false) {
		    item = (struct item *)emalloc(sizeof(struct item));
		    item->name = curr->name;
		    LIST_INSERT_HEAD(&prev->head, item, entries);
	       }
	  }

	  prev = curr;
     }
     
     /* Function pointer is assigned only for the last token */
     curr->get = path->get;
}

void paths_init()
{
     int i;

     for (i = 0; i < NPATHS; i++) {
	  path_add(&paths[i]);
     }
}

/* Return the B-tree node corresponding to the last token of a valid path */

struct node *path_lookup(char *tokens[])
{
     int i;
     struct node *node = NULL;
     struct node key = { 0 };
     void *val;
     struct item *item;
     bool found;

     for (i = 0; i < PATH_MAX_TOKENS && tokens[i] != NULL; i++) {
	  key.name = tokens[i];
	  val = tfind((void *)&key, &root, compar);

	  /* Path token can't be found */
	  if (val == NULL)
	       break;

	  node = *(struct node **)val;

	  /* This is the last token in the path */
	  if (i == PATH_MAX_TOKENS - 1 || tokens[i + 1] == NULL) {
	       if (node->get == NULL)
		    node = NULL;
	       break;
	  }

	  /* Another token follows: it must be part of the adjacency list. */
	  found = false;
	  for (item = node->head.lh_first; item != NULL;
	       item = item->entries.le_next) {
	       if (strcmp(item->name, tokens[i + 1]) == 0 ) {
		    found = true;
		    break;
	       }
	  }
	  if (found == false) {
	       node = NULL;
	       break;
	  }
     }

     return node;
}

static void print_name(const void *p, const VISIT which, const int depth)
{
     struct node *node;
     struct item *item;

     switch (which) {

     case preorder:			/* before visiting the  children */
	  break;

     case postorder:			/* after the 1st and before the 2nd */
	  node = *(struct node **)p;
	  for (item = node->head.lh_first; item != NULL;
	       item = item->entries.le_next) {
	       printf("    %s -> %s;\n", node->name, item->name);
	  }
	  break;

     case endorder:			/* after visiting the children */
 	  break;

     case leaf:
	  node = *(struct node **)p;
	  /* B-tree leaf doesn't mean graph leaf !!! */
	  for (item = node->head.lh_first; item != NULL;
	       item = item->entries.le_next) {
	       printf("    %s -> %s;\n", node->name, item->name);
	  }
	  break;
     }
}

int main()
{
     int i;
     struct node *node;

     paths_init();

     printf("digraph G {\n");
     twalk(root, print_name);
     printf("}\n");

     for (i = 0; i < NPATHS; i++) {
	  node = path_lookup(paths[i].tokens);
	  if (node != NULL)
	       node->get();
     }

     for (i = 0; i < NDUMMIES; i++) {
	  node = path_lookup(dummies[i].tokens);
	  if (node != NULL)
	       node->get();
     }

     exit(EXIT_SUCCESS);
}
