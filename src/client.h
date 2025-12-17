#ifndef CLIENT_H
#define CLIENT_H

#include "context.h"

int client_start(struct context *ctx, const char *path, int port);

#endif /* CLIENT_H */
