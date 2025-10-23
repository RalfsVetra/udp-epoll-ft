#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "server.h"
#include "client.h"

#define NON_PRIVILEGED_PORT 9367

static int server_call(unsigned port)
{
	struct server *svr = server_init(port);
	if (!svr)
		return EXIT_FAILURE;
	
	server_start(svr);
	return EXIT_SUCCESS;
}

static int client_call()
{
        return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
	if (argc == 2 && strcmp(argv[1], "server") == 0)
		return server_call(NON_PRIVILEGED_PORT);

	if (argc == 3 && strcmp(argv[1], "client") == 0)
		return client_call();
	
	printf("Incorrect usage. Please refer to README.md");
	return EXIT_FAILURE;
}
