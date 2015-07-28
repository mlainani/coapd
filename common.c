#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

const char *program_name;

void *
xmalloc (size_t size)
{
  void *ptr = malloc (size);
  /* Abort if the allocation failed.  */
  if (ptr == NULL)
    abort ();
  else
    return ptr;
}

void
error (const char *cause, const char *message)
{
  /* Print an error message to stderr.  */
  fprintf (stderr, "%s: error: (%s) %s\n", program_name, cause, message);
  /* End the program.  */
  exit (1);
}

void
system_error (const char *operation)
{
  /* Generate an error message for errno.  */
  error (operation, strerror (errno));
}

void
warning (const char *message)
{
  /* Print an error message to stderr.  */
  fprintf (stderr, "%s: warning: %s\n", program_name, message);
}
