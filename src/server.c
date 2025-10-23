#include <stdint.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include "server.h"
#include "common.h"
#include "packet.h"

static int create_listen(unsigned port)
{
	int sockfd = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0);
	if (sockfd == -1)
		die("socket");
	
	struct sockaddr_in addr = {0};
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);

	if (bind(sockfd, (struct sockaddr *)&addr, sizeof addr) == -1) {
		close(sockfd);
		die("bind");
	}
	return sockfd;
}

struct server *server_init(unsigned port)
{
	struct server *svr = calloc(1, sizeof *svr);
	if (svr == NULL)
		die("calloc");

	svr->epfd = epoll_create1(0);
	if (svr->epfd == -1) {
		free(svr);
		die("epoll_create1");
	}

	svr->listen = create_listen(port);

	struct epoll_event ev = {0};
	ev.events = EPOLLIN;
	ev.data.fd = svr->listen;

	if (epoll_ctl(svr->epfd, EPOLL_CTL_ADD, svr->listen, &ev) == -1) {
		close(svr->listen);
		close(svr->epfd);
		free(svr);
		die("epoll_ctl");
	}
	return svr;
}

static void parse_message(const uint8_t *buf, size_t len,
			const struct sockaddr *client, socklen_t client_len)
{
	switch (buf[0]) {
	case PKT_START:
		handle_start((const struct pkt_start *)buf, len, client, client_len);
		break;
	case PKT_PAYLOAD:
		handle_payload((const struct pkt_payload *)buf, len, client, client_len);
		break;
	case PKT_DONE:
		handle_done((const struct pkt_done *)buf, len, client, client_len);
		break;
	default:
		break;
	}
}

static void server_handle(struct server *svr)
{
	for (;;) {
		uint8_t buf[TWO_KB];
		struct sockaddr_storage client;
		socklen_t client_len = sizeof(client);

		ssize_t len = recvfrom(svr->listen, buf, sizeof buf, 0, (struct sockaddr *)&client, &client_len);
		if (len == -1) {
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				break;
				
			if (errno == EINTR)
				continue;
			die("recvfrom");
		}

		if (len == 0)
			continue;
	
		parse_message(buf, (size_t)len, (struct sockaddr *)&client, client_len);
	}
}

void server_start(struct server *svr)
{
	struct epoll_event events[MAX_EVENTS];

	for (;;) {
		int ready = epoll_wait(svr->epfd, events, MAX_EVENTS, -1);
		if (ready == -1) {
			if (errno == EINTR)
				continue;
			die("epoll_wait");
		}

		if (ready == 0)
			continue;

		for (int i = 0; i < ready; i++) {
			if (events[i].data.fd != svr->listen)
				continue;

			if (events[i].events & EPOLLIN)
				server_handle(svr);
		}
	}
}

void server_stop(struct server *svr)
{
	if (!svr)
		return;
	
	if (svr->listen >= 0)
		close(svr->listen);
	
	if (svr->epfd >= 0)
		close(svr->epfd);
	free(svr);
}

