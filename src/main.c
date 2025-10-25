#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include "server.h"
#include "client.h"
#include "context.h"

#define PRIVILEGED_PORT 1024
#define MAX_PORT        65535

static int parse_port(const char *port_str)
{
    char *end = NULL;
    long port = strtol(port_str, &end, 10);

    if (*end != '\0' || errno == ERANGE || end == port_str)
	return -1;

    if (port < PRIVILEGED_PORT || port > MAX_PORT)
	return -1;
	
    return (int)port;
}

static int is_valid_path(const char *path)
{
    return access(path, F_OK | W_OK) == 0 ? 0 : -1;
}

static int server_call(int port)
{
    struct context *ctx = context_init(port);
    if (!ctx)
        return EXIT_FAILURE;

    int rc = server_start(ctx);
    context_close(ctx);
    return (rc == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

static int client_call(int port, const char *path)
{
    struct context *ctx = context_init(port);
    if (!ctx)
        return EXIT_FAILURE;

    int rc = client_start(ctx, path);
    context_close(ctx);
    return (rc == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

static void usage(void)
{
    fprintf(stderr, "Incorrect usage. Please refer to README.md\n");
}

int main(int argc, char *argv[])
{
    if (argc == 3 && strcmp(argv[1], "server") == 0) {
	int port = parse_port(argv[2]);
	if (port != -1)
	    return server_call(port);
    }

    if (argc == 4 && strcmp(argv[1], "client") == 0) {
	int port = parse_port(argv[2]);
	if (port != -1 && is_valid_path(argv[3]) != -1)
	    return client_call(port, argv[3]);
    }

    usage();
    return EXIT_FAILURE;
}

