#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include "common.h"

void die(char *s)
{
	perror(s);
	exit(EXIT_FAILURE);
}

void set_nonblocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1)
		die("fcntl(F_GETFL)");

	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK))
		die("fcntl(F_SETFL)");
}
