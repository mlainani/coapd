#include <string.h>

#include "message.h"
#include "option.h"
#include "server.h"

static struct option options[] = {
  {COAP_OPT_IF_MATCH, true, 0, 8, "If-Match"},			/* opaque */
  {COAP_OPT_URI_HOST, false, 1, 255, "Uri-Host"},		/* string */		      
  {COAP_OPT_ETAG, true, 1, 8, "ETag"},				/* opaque */
  {COAP_OPT_IF_NONE_MATCH, false, 0, 0, "If-None-Match"},	/* empty */
  {COAP_OPT_URI_PORT, false, 0, 2, "Uri-Port"},			/* uint */
  {COAP_OPT_URI_PATH, true, 0, 255, "Uri-Path"},		/* string */
  {COAP_OPT_URI_QUERY, true, 0, 255, "Uri-Query"},		/* string */
  {COAP_OPT_ACCEPT, false, 0, 2, "Accept"},			/* uint */
  {COAP_OPT_PROXY_URI, false, 1, 1034, "Proxy-Uri"},		/* string */
  {COAP_OPT_PROXY_SCHEME, false, 1, 255, "Proxy-Scheme"},	/* string */
};

#define nr_of_options ( sizeof(options) / sizeof(options[0]) )

static int
compare (const void *p1, const void *p2)
{
  struct option *opt1 = (struct option *) p1;
  struct option *opt2 = (struct option *) p2;

  if (opt1->num < opt2->num)
    return -1;
  if (opt1->num > opt2->num)
    return 1;
  return 0;
}

int
coap_req_get (struct message * msg)
{
  uint8_t *opt, *optval;
  size_t offset, left, len, hdrlen;
  uint16_t delta, optlen, optnum;
  struct option key, *prev = NULL;
  char str[256];		/* terminating null byte */

  opt = msg->data + COAP_HDR_SIZE + msg->tklen;
  offset = 0;
  left = len = msg->len - COAP_HDR_SIZE - msg->tklen;

  while (offset < len)
    {
      hdrlen = option_parse (opt, left, &delta, &optlen);
      if (hdrlen < 0)
	{
	  //request_free ();
	  return -1;
	}

      offset += hdrlen;
      left -= hdrlen;
      opt += hdrlen;

      if (delta == PAYLOAD_MKR)
	{
	  goto payload;
	}

      if (delta == 0 && prev != NULL && prev->repeat == false)
	{
	  system_log ("repeated %s\n", prev->name);
	  return -1;
	}

      optnum = (prev == NULL ? 0 : prev->num) + delta;
      key.num = optnum;
      prev = bsearch (&key, options, nr_of_options,
		      sizeof (struct option), compare);
      if (prev == NULL)
	{
	  system_log ("'%d': invalid option\n", key.num);
	  return -1;
	}
      else
	{
	  system_log ("%s: option #%d\n", prev->name, prev->num);
	  ;
	}
      optval = opt + hdrlen;

      switch (optnum)
	{
	case COAP_OPT_URI_PATH:
	  strncpy (str, (char *) optval, optlen);
	  str[optlen] = '\0';
	  system_log ("%s: str %s\n", prev->name, str);
	  break;
	default:
	  break;
	}

      offset += optlen;
      left -= optlen;
      opt += optlen;
    }

 payload:
  ;
  return 0;
}
