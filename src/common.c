#include <stdlib.h>
#include <stdio.h>
#include "common.h"

void die(char *s)
{
	perror(s);
	exit(EXIT_FAILURE);
}

