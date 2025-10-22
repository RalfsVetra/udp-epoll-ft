#include <stdlib.h>
#include <sys/epoll.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include "server.h"
#include "common.h"
#include "packet.h"

static int create_listen(unsigned port, struct server *svr)
{
	struct sockaddr_in addr;

	memset(&addr, 0, sizeof addr);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);

	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd == -1) {
		free(svr);
		die("socket");
	}

	set_nonblocking(sockfd);

	if (bind(sockfd, (struct sockaddr*) &addr, sizeof(struct sockaddr)) == -1) {
		free(svr);
		die("bind");
	}
	return sockfd;
}

struct server *server_init(unsigned port)
{
	struct server *svr = NULL;
	struct epoll_event ev;

	svr = calloc(1, sizeof(struct server));
	if (svr == NULL)
		die("calloc");

	svr->epfd = epoll_create1(0);
	if (svr->epfd == -1) {
		free(svr);
		die("epoll_create1");
	}

	svr->listen = create_listen(port, svr);

	ev.events = EPOLLIN;
	ev.data.fd = svr->listen;

	if (epoll_ctl(svr->epfd, EPOLL_CTL_ADD, svr->listen, &ev) == -1) {
		free(svr);
		die("epoll_ctl");
	}
	return svr;
}

static void parse_message(char buf[])
{
	uint8_t packet_type = buf[0];
	switch (packet_type) {
	case PKT_START:
		break;
	case PKT_PAYLOAD:
		break;
	case PKT_DONE:
		break;
	default:
		break;
	}
}

static void server_handle(struct server *svr)
{
	char buf[TWO_KB] = {0};
	struct sockaddr_storage addr;
	socklen_t addr_len = sizeof(struct sockaddr);

	ssize_t len = recvfrom(svr->listen, buf, sizeof buf, 0, (struct sockaddr*) &addr, &addr_len);
	if (len == -1 && errno != EAGAIN && errno != EWOULDBLOCK)
		die("recvfrom");
	
	if (len > 0)
		parse_message(buf);
}

void server_start(struct server *svr)
{
	struct epoll_event events[MAX_EVENTS];

	for (;;) {
		int ready = epoll_wait(svr->epfd, events, MAX_EVENTS, -1);
		if (ready == -1)
			die("epoll_wait");

		if (ready == 0)
			continue;

		for (int i = 0; i < ready; i++) {
			if (events[i].data.fd != svr->listen)
				continue;

			server_handle(svr);
		}
	}
}

void server_stop(struct server *svr)
{
	close(svr->epfd);
	free(svr);
}

