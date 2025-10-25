#ifndef _SERVER_H
#define _SERVER_H

#define MAX_EVENTS 100
#define TWO_KB     (2 * 1024)

#include "context.h"

int server_start(struct context *ctx);

#endif /* _SERVER_H */
