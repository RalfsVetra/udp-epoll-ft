#include <stdlib.h>
#include <sys/epoll.h>
#include <unistd.h>
#include "context.h"
#include "net.h"

void context_close(struct context *ctx)
{
    if (!ctx)
	return;
	
    if (ctx->sock_fd >= 0)
	close(ctx->sock_fd);
	
    if (ctx->epoll_fd >= 0)
	close(ctx->epoll_fd);
    free(ctx);
}
 
struct context *context_init(int port)
{
    struct context *ctx = calloc(1, sizeof *ctx);
    if (!ctx)
	return NULL;

    ctx->epoll_fd = epoll_create1(0);
    if (ctx->epoll_fd == -1) {
	free(ctx);
	return NULL;
    }

    ctx->sock_fd = net_create_udp_listener(port);
    if (ctx->sock_fd == -1) {
	context_close(ctx);
	return NULL;
    }

    struct epoll_event ev = {0};
    ev.events = EPOLLIN;
    ev.data.fd = ctx->sock_fd;

    if (epoll_ctl(ctx->epoll_fd, EPOLL_CTL_ADD, ctx->sock_fd, &ev) == -1) {
	context_close(ctx);
	return NULL;
    }
    return ctx;
}

