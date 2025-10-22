#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "server.h"
#include "client.h"

#define NON_PRIVILEGED_PORT 1024u

static int server_call()
{
	struct server *svr;
	svr = server_init(NON_PRIVILEGED_PORT);
	server_start(svr);
	server_stop(svr);
	return EXIT_SUCCESS;
}

static int client_call()
{
        return 0;
}

int main(int argc, char *argv[])
{
	if (argc == 2 && strcmp(argv[1], "server") == 0) {
		server_call();
	} else if (argc == 3 && strcmp(argv[1], "client") == 0) {
		client_call();
	} else {
		printf("Incorrect usage. Please refer to README.md");
	}
	return EXIT_SUCCESS;
}
