#ifndef _CLIENT_H
#define _CLIENT_H

#define ONE_KB (1 * 1024)

#include "context.h"

int client_start(struct context *ctx, const char *path, int port);

#endif /* _CLIENT_H */
