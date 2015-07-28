#include <netinet/in.h>		/* sockaddr_in6 */
#include <stdlib.h>		/* malloc() */
#include <string.h>		/* memset() */

#include "message.h"
#include "server.h"

#define COAP_MSG_MIN_SIZE	4

/* RFC 7252 - Section 4.6 */
#define COAP_MSG_MAX_SIZE	1152

#define COAP_DEFAULT_PORT	5683

void
server_run ()
{
  int sockfd;
  struct sockaddr_in6 addr;
  socklen_t addrlen;
  struct message *msg;
  int ret;

  sockfd = socket (AF_INET6, SOCK_DGRAM, 0);
  if (sockfd == -1)
    system_error ("socket");
  memset (&addr, 0, sizeof (addr));
  addr.sin6_family = AF_INET6;
  addr.sin6_port = htons (COAP_DEFAULT_PORT);
  addr.sin6_addr = in6addr_any;
  if (bind (sockfd, (struct sockaddr *) &addr, sizeof (addr)) == -1)
    system_error ("bind");

  addrlen = sizeof(struct sockaddr_in6);

  while (1)
    {
      msg = coap_message_new ();
      msg->len = recvfrom (sockfd, msg->data, COAP_MSG_MAX_SIZE, 0,
			   (struct sockaddr *) &msg->src, &addrlen);

      if (msg->len > 0)
	{
	  if (msg->len < COAP_MSG_MIN_SIZE)
	    {
	      /* Should be at least 4-byte long */
	      warning ("message too short");
	      coap_message_free (msg);
	    }
	  else
	    {
	      ret = coap_header_parse (msg);
	      if (ret < 0)
		{
		  warning("invalid message header");
		  coap_message_free (msg);
		}
	    }
	}
      else
	system_error ("recvfrom");
    }
}
