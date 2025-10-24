#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "server.h"
#include "client.h"

#define PRIVILEGED_PORT 1024
#define MAX_PORT        65535

static int server_call(int port)
{
	struct server *svr = server_init(port);
	if (!svr)
		return EXIT_FAILURE;

	server_start(svr);
	return EXIT_SUCCESS;
}

static int client_call(int port, char *path)
{
	/* struct client *clt = client_init(port); */
	/* if (!clt) */
	/* 	return EXIT_FAILURE; */

	/* client_start(clt); */
	return EXIT_SUCCESS;
}

static void usage(void)
{
	fprintf(stderr, "Incorrect usage. Please refer to README.md\n");
}

static int validate_port(const char *port_str)
{
	char *end;
	long port = strtol(port_str, &end, 10);

	if (*end != '\0' || errno == ERANGE)
		return EXIT_FAILURE;

	if (port < PRIVILEGED_PORT || port > MAX_PORT)
		return EXIT_FAILURE;
	
	return (int)port;
}

static void validate_path()
{
	return;
}

int main(int argc, char *argv[])
{
	if (argc == 3 && strcmp(argv[1], "server") == 0) {
		int port = validate_port(argv[2]);
		if (port != EXIT_FAILURE)
			return server_call(port);
	}

	if (argc == 4 && strcmp(argv[1], "client") == 0) {
		int port = validate_port(argv[2]);
		if (port != EXIT_FAILURE)
			return client_call(port, argv[3]);
	}

	usage();
	return EXIT_FAILURE;
}

