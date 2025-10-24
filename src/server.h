#ifndef _SERVER_H
#define _SERVER_H

#define MAX_EVENTS 100
#define TWO_KB     (2 * 1024)

struct server {
	int epfd;
	int listen;
};

struct server *server_init(int port);
void server_start(struct server *svr);

#endif /* _SERVER_H */
