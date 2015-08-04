#ifndef SERVER_H
#define SERVER_H

#include <pthread.h>
#include <sys/types.h>

/* The name of this program.  */
extern const char *program_name;

/* Like malloc, except aborts the program if allocation fails.  */
extern void *xmalloc (size_t size);

/* Print an error message for a failed call OPERATION, using the value
   of errno, and end the program.  */
extern void system_error (const char *operation);

/* Print an error message for failure involving CAUSE, including a
   descriptive MESSAGE, and end the program.  */
extern void error (const char *cause, const char *message);

/* Print a warning message */
extern void warning (const char *message);

/* Print a log message */
extern void system_log (const char *fmt, ...);

extern void server_run ();

#endif /* SERVER_H */
