#include "linux/limits.h"

#include "coap.h"

int main()
{
     printf("Linux max path length: %d bytes\n", PATH_MAX);
     exit(0);
}
