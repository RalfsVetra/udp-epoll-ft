#include <stdint.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <errno.h>
#include "server.h"
#include "packet.h"
#include "context.h"
#include "transfer.h"

static void send_ack(int sock_fd, const uint8_t transfer_id[16], int done,
		     const struct sockaddr *client, socklen_t client_len)
{
    struct pkt_ack a;
    a.type = done == 0 ? PKT_OK : PKT_ERROR;
    memcpy(a.transfer_id, transfer_id, 16);

    (void)sendto(sock_fd, &a, sizeof a, 0, client, client_len);
}

static void parse_message(struct context *ctx, const uint8_t *buf, size_t len,
			  const struct sockaddr *client, socklen_t client_len)
{
    if (len == 0)
	return;
	
    switch (buf[0]) {
    case PKT_START:
	handle_start((const struct pkt_start *)buf, len, client, client_len);
	break;
	
    case PKT_PAYLOAD:
	handle_payload((const struct pkt_payload *)buf, len, client, client_len);
	break;
	
    case PKT_DONE:
	const struct pkt_done *d = (const struct pkt_done *)buf;
	int done = handle_done(d, len);
	send_ack(ctx->sock_fd, d->transfer_id, done, client, client_len);
	break;
	
    default:
	break;
    }
}

static int server_handle(struct context *ctx)
{
    for (;;) {
	uint8_t buf[TWO_KB];
	struct sockaddr_storage client;
	socklen_t client_len = sizeof client;

	ssize_t len = recvfrom(ctx->sock_fd, buf, sizeof buf, 0,
			       (struct sockaddr *)&client, &client_len);
	if (len == -1) {
	    if (errno == EINTR)
		continue;
	    
	    if (errno == EAGAIN || errno == EWOULDBLOCK)
		break;
	    return -1;
	}

	if (len == 0)
	    continue;
	
	parse_message(ctx, buf, (size_t)len, (struct sockaddr *)&client, client_len);
    }
    return 0;
}

int server_start(struct context *ctx)
{
    struct epoll_event events[MAX_EVENTS];

    for (;;) {
	int ready = epoll_wait(ctx->epoll_fd, events, MAX_EVENTS, -1);
	if (ready == -1) {
	    if (errno == EINTR)
		continue;
	    return -1;
	}

	if (ready == 0)
	    continue;

	for (int i = 0; i < ready; i++) {
	    uint32_t ev = events[i].events;
	    int fd = events[i].data.fd;
	    
	    if (fd != ctx->sock_fd)
		continue;

	    if (ev & (EPOLLERR | EPOLLHUP))
                return -1;

	    if (ev & EPOLLIN)
		if (server_handle(ctx) == -1)
		    return -1;
	}
    }
}

