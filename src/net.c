#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "net.h"

void udp_make_loopback_addr(struct sockaddr_in *addr, int port) {
    memset(addr, 0, sizeof *addr);
    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr->sin_port = htons((uint16_t)port);
}

int udp_create_bound_socket(int port)
{
    int sock_fd = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0);
    if (sock_fd == -1)
		return -1;
	
	struct sockaddr_in addr;
	udp_make_loopback_addr(&addr, port);
	
    if (bind(sock_fd, (struct sockaddr *)&addr, sizeof addr) == -1) {
		close(sock_fd);
		return -1;
    }
    return sock_fd;
}

int udp_sendto(int sock_fd, const void *buf, size_t len,
               const struct sockaddr *addr, socklen_t addrlen)
{
    for (;;) {
        ssize_t n = sendto(sock_fd, buf, len, 0, addr, addrlen);
        if (n == (ssize_t)len)
			return 0;
		
        if (n == -1 && (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK))
            continue;
        return -1;
    }
	return 0;
}
