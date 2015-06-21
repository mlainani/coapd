#include <stdint.h>	/* uintN_t */
#include <stdio.h>	/* printf */

int handle_get(uint16_t mid)
{
     printf("%s was invoked\n", __func__);
     return 0;
}
