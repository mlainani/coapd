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

enum { PATH_MAX_TOKENS = 4 };

struct path {
     int (*get)();
     char *tokens[PATH_MAX_TOKENS];
} paths[] = {
     { some_get, { "path", "to", "some" } },
     { other_get, { "path", "to", "other" } },
};

#define NPATHS ( sizeof(paths) / sizeof(paths[0]) )

/* Linked list item - path graph vertex adjacency list element */
struct item {
     char *name;
     LIST_ENTRY(item) entries;
};

/* Binary tree node - path graph vertex */
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
	  str != NULL;
	  str = path->tokens[++i]) {

	  /* 
           * Allocate and initialize the b-tree node element. Needed even if we
	   * just want to search a node.
           */

	  curr = (struct node *)emalloc(sizeof(struct node));
	  curr->name = str;

	  /* 
           * Add the node to the b-tree if it doesn't exist yet. If it exists
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
		    if (curr->name == item->name)
			 found = true;
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

/* Return the b-tree node corresponding to the last token of a valid path */

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
	       if (strcmp(item->name, tokens[i + 1]) == 0 )
		    found = true;
	  }
	  if (found == false) {
	       node = NULL;
	       break;
	  }
     }

     return node;
}

int main()
{
     int i;
     struct node *node;

     paths_init();

     for (i = 0; i < NPATHS; i++) {
	  node = path_lookup(paths[i].tokens);
	  if (node != NULL)
	       node->get();
     }

     exit(EXIT_SUCCESS);
}
