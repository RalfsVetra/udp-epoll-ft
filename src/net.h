#ifndef NET_H
#define NET_H

#include <netinet/in.h>
#include <sys/socket.h>

void udp_make_loopback_addr(struct sockaddr_in *addr, int port);
int udp_create_bound_socket(int port);
int udp_sendto(int sock_fd, const void *buf, size_t len,
               const struct sockaddr *addr, socklen_t addrlen);

#endif /* NET_H */
