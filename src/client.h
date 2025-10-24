#ifndef _CLIENT_H
#define _CLIENT_H

#define ONE_KB (1 * 1024)

struct client {
	int epfd;
	int listen;
};

struct client *client_init(int port);
void client_start(struct client *clt);

#endif /* _CLIENT_H */
