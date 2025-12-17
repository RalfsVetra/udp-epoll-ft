#ifndef CONTEXT_H
#define CONTEXT_H

struct context {
    int sock_fd;
    int epoll_fd;
};

struct context *context_init(int port);
void context_close(struct context *ctx);

#endif /* CONTEXT_H */
