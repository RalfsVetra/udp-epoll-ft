#ifndef _NET_H
#define _NET_H

#include <netinet/in.h>

int create_udp_listener(int port);
void make_loopback_addr(struct sockaddr_in *addr, int port);

#endif /* _NET_H */
