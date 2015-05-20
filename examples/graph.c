#define _GNU_SOURCE
#include <search.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/queue.h>

#include "linux/limits.h"

/* 
 * Redirect program output to Graphviz file foobar.gv and generate the
 * graph picture file with
 * 
 * 	dot -Tsvg foobar.gv -o foobar.svg
 */

struct myentry {
     char *name;
     LIST_ENTRY(myentry) entries;
};

/* My data item for the tree nodes */
struct vertex {
     char *name;
     LIST_HEAD(listhead, myentry) head;
};

static void *root = NULL;

static int compar(const void *p1, const void *p2)
{
     struct vertex *v1 = (struct vertex *)p1;
     struct vertex *v2 = (struct vertex *)p2;

     return strcmp(v1->name, v2->name);
}

/* 
 * The first field in each node of the tree is a pointer to the
 * corresponding data item. (The calling program must store the actual
 * data.)
 */

static void print_name(const void *nodep, const VISIT which, const int depth)
{
     struct vertex *v;
     struct myentry *np;

     switch (which) {

     case preorder:			/* before visiting the  children */
#if 0
	  v = *(struct vertex **)nodep;
	  printf("%s\n", v->name);
#endif
	  break;

     case postorder:			/* after the 1st and before the 2nd */
#if 1
	  v = *(struct vertex **)nodep;
	  /* printf("    %s", v->name); */
	  for (np = v->head.lh_first; np != NULL; np = np->entries.le_next) {
	       printf("    %s -> %s;\n", v->name, np->name);
	  }
	  /* printf(";\n"); */
#endif
	  break;

     case endorder:			/* after visiting the children */
#if 0
	  v = *(struct vertex **)nodep;
	  printf("%s\n", v->name);
#endif
 	  break;

     case leaf:
#if 1
	  v = *(struct vertex **)nodep;
	  /* printf("    %s", v->name); */
	  /* B-tree leaf doesn't mean graph leaf !!! */
	  for (np = v->head.lh_first; np != NULL; np = np->entries.le_next) {
	       printf("    %s -> %s;\n", v->name, np->name);
	  }
	  /* printf(";\n"); */
#endif
	  break;
     }
}

void free_node(void *nodep)
{
     struct vertex *v;

     v = (struct vertex *)nodep;

     if (v != NULL) {
	  
	  if (v->name != NULL)
	       free(v->name);

	  while (v->head.lh_first != NULL) {
	       if (v->head.lh_first->name != NULL)
		    free(v->head.lh_first->name);
	       LIST_REMOVE(v->head.lh_first, entries);
	  }
	  
	  free(v);
     }
}

int main(int argc, char *argv[])
{
     int i;
     char path[PATH_MAX], *str;
     char *token;
     struct vertex *v, *prev;
     void *val;
     struct myentry *np;
     bool first, found;

     if (argc > 1) {

	  for (i = 1; i < argc; i++) {

	       strcpy(path, argv[i]);
	  
	       printf("\"%s\" has %lu chars\n", path, strlen(path));

	       for (first = true, prev = NULL, str = &path[0]; ; str = NULL) {

		    token = strtok(str, "/");

		    /* We've reached the end of the string */
		    if (token == NULL)
			 break;
	       
		    /* Allocate and initialize the b-tree node element*/
		    /* Needed even if we just want to search for a node */
		    v = malloc(sizeof(struct vertex));
		    if (v == NULL)
			 goto destroy;

		    LIST_INIT(&v->head);

		    v->name = strdup(token);
		    if (v->name == NULL) {
			 free(v);
			 goto destroy;
		    }

		    /* Add the node element to the b-tree if it doesn't exist yet*/
		    val = tfind((void *)v, &root, compar);
		    if (val != NULL) {
			 free(v->name);
			 free(v);
			 prev = *(struct vertex **)val;
			 first = false;
			 continue;
		    }

		    val = tsearch((void *)v, &root, compar);
		    if (val == NULL)
			 goto destroy;

		    /* Add the current token to it's parent adjacency list  */
		    if (first == false) {

			 found = false;

			 for (np = prev->head.lh_first; np != NULL; np = np->entries.le_next) {
			      if (strcmp(v->name, np->name) == 0)
				   found = true;
			 }

			 if (found == false) {

			      np = malloc(sizeof(struct myentry));
			      if (np == NULL)
				   goto destroy;

			      np->name = strdup(v->name);
			      if (np->name == NULL) {
				   free(np);
				   goto destroy;
			      }

			      LIST_INSERT_HEAD(&prev->head, np, entries);
			 }
		    }

		    first = false;
		    prev = v;
	       }
	  }
	  
	  printf("digraph G {\n");
	  twalk(root, print_name);
	  printf("}\n");
	  goto destroy;
     }

     while (fgets(path, sizeof(path), stdin)) {

	  /* Get rid of the new line character */
	  path[strlen(path) - 1] = '\0';
	  
	       for (first = true, prev = NULL, str = &path[0]; ; str = NULL) {

		    token = strtok(str, "/");

		    /* We've reached the end of the string */
		    if (token == NULL)
			 break;
	       
		    /* Allocate and initialize the b-tree node element*/
		    /* Needed even if we just want to search for a node */
		    v = malloc(sizeof(struct vertex));
		    if (v == NULL)
			 goto destroy;

		    LIST_INIT(&v->head);

		    v->name = strdup(token);
		    if (v->name == NULL) {
			 free(v);
			 goto destroy;
		    }

		    /* Add the node element to the b-tree if it doesn't exist yet*/
		    val = tfind((void *)v, &root, compar);
		    if (val != NULL) {
			 free(v->name);
			 free(v);
			 prev = *(struct vertex **)val;
			 first = false;
			 continue;
		    }

		    val = tsearch((void *)v, &root, compar);
		    if (val == NULL)
			 goto destroy;

		    /* Add the current token to it's parent adjacency list  */
		    if (first == false) {

			 found = false;

			 for (np = prev->head.lh_first; np != NULL; np = np->entries.le_next) {
			      if (strcmp(v->name, np->name) == 0)
				   found = true;
			 }

			 if (found == false) {

			      np = malloc(sizeof(struct myentry));
			      if (np == NULL)
				   goto destroy;

			      np->name = strdup(v->name);
			      if (np->name == NULL) {
				   free(np);
				   goto destroy;
			      }

			      LIST_INSERT_HEAD(&prev->head, np, entries);
			 }
		    }

		    first = false;
		    prev = v;
	       }
     }

     printf("digraph G {\n");
     twalk(root, print_name);
     printf("}\n");

destroy:
     tdestroy(root, free_node);

     exit(EXIT_SUCCESS);
}
