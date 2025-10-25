#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include "net.h"

void make_loopback_addr(struct sockaddr_in *addr, int port) {
    memset(addr, 0, sizeof *addr);
    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr->sin_port = htons(port);
}

int create_udp_listener(int port)
{
    int sock_fd = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0);
    if (sock_fd == -1)
		return -1;
	
	struct sockaddr_in addr;
	make_loopback_addr(&addr, port);
	
    if (bind(sock_fd, (struct sockaddr *)&addr, sizeof addr) == -1) {
		close(sock_fd);
		return -1;
    }
    return sock_fd;
}

