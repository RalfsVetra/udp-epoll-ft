#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/epoll.h>
#include <stdio.h>
#include "client.h"
#include "context.h"
#include "packet.h"
#include "net.h"
#include "util.h"

#define ONE_KB     1024
#define MAX_EVENTS 1

static int send_pkt_start(int sock_fd, const uint8_t transfer_id[16], const char *path,
						  int32_t file_size, const struct sockaddr *addr, socklen_t addrlen)
{
    uint8_t buf[sizeof(struct pkt_start) + ONE_KB];
    int n = build_pkt_start(buf, sizeof buf, transfer_id, path, file_size);
    if (n == -1)
		return -1;
    return udp_sendto(sock_fd, buf, (size_t)n, addr, addrlen);
}

static int send_pkt_payload(int sock_fd, const uint8_t transfer_id[16],
                            uint32_t offset, const uint8_t *data, uint16_t len,
							const struct sockaddr *addr, socklen_t addrlen)
{
    uint8_t buf[sizeof(struct pkt_payload) + ONE_KB];
    int n = build_pkt_payload(buf, sizeof buf, transfer_id, offset, data, len);
    if (n == -1)
		return -1;
    return udp_sendto(sock_fd, buf, (size_t)n, addr, addrlen);
}

static int send_pkt_done(int sock_fd, const uint8_t transfer_id[16],
						 const struct sockaddr *addr, socklen_t addrlen)
{
    uint8_t buf[sizeof(struct pkt_done)];
    int n = build_pkt_done(buf, transfer_id);
    if (n == -1)
        return -1;
    return udp_sendto(sock_fd, buf, (size_t)n, addr, addrlen);
}

static void print_progress(uint32_t sent, uint32_t total)
{
    if (total == 0)
        return;
    
    const int bar_width = 40;
    static int start_time = 0;

    if (start_time == 0)
        start_time = now_sec();

    int diff = now_sec() - start_time;
    int time_elapsed = diff > 0 ? diff : 1;
    
    int mbps = sent / time_elapsed / (1024 * 1024);
    float progress = (float) sent / total;
    int bar_length = progress * bar_width;

    printf("\r[");
    for (int i = 0; i < bar_length; ++i) {
        printf("=");
    }

    for (int i = bar_length; i < bar_width; ++i) {
        printf(" ");
    }
    printf("] %.0f%% %dMB/s", progress * 100, mbps);
    fflush(stdout);

    if (sent == total) {
        printf("\n");
        fflush(stdout);
    }
}

static int send_file_payloads(int sock_fd, const uint8_t transfer_id[16], int fd,
							  int32_t file_size, const struct sockaddr *addr,
							  socklen_t addrlen)
{
    uint8_t buf[ONE_KB];
    uint32_t offset = 0;

    for (;;) {
        ssize_t n = read(fd, buf, sizeof buf);
        if (n > 0) {
            if (send_pkt_payload(sock_fd, transfer_id, offset,
								 buf, (uint16_t)n, addr, addrlen) == -1)
                return -1;
            
            offset += (uint32_t)n;
            print_progress(offset, (uint32_t)file_size);
            continue;
        }
        
        if (n == 0)
			break;
		
        if (errno == EINTR)
			continue;
        return -1;
    }
    return 0;
}

static int handle_ack(struct context *ctx, const uint8_t transfer_id[16])
{
    for (;;) {
        uint8_t buf[ONE_KB];
        struct sockaddr_storage src;
        socklen_t src_len = sizeof src;

        ssize_t len = recvfrom(ctx->sock_fd, buf, sizeof buf, 0,
                               (struct sockaddr *)&src, &src_len);
        if (len == -1) {
            if (errno == EINTR)
                continue;

            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;
            return -1;
        }

        if (len == 0)
            continue;

        const struct pkt_ack *ack = (const struct pkt_ack *)buf;
        if (memcmp(ack->transfer_id, transfer_id, 16)!= 0)
            continue;

        return (ack->type == PKT_OK) ? 0 : -1;
    }
    
    return 0;
}

static int wait_ack(struct context *ctx, const uint8_t transfer_id[16])
{
    struct epoll_event event[MAX_EVENTS];

    for (;;) {
        printf("Waiting for ack...\n");
        int ready = epoll_wait(ctx->epoll_fd, event, MAX_EVENTS, 5000);
        if (ready == -1) {
            if (errno == EINTR)
                continue;
            return -1;
        }

        if (ready == 0)
            return -1;

        int fd = event[0].data.fd;
        uint32_t ev = event[0].events;

        if (fd != ctx->sock_fd)
            continue;

        if (ev & (EPOLLERR | EPOLLHUP))
            return -1;

        if (ev & EPOLLIN)
            return handle_ack(ctx, transfer_id);
    }
    return 0;
}

int client_start(struct context *ctx, const char *path, int port)
{
	uint8_t transfer_id[16] = {0};
	arc4random_buf(transfer_id, sizeof transfer_id);

	int fd = open(path, O_RDONLY);
	if (fd == -1)
		return -1;

	int32_t file_size = get_file_size32(path);
	if (file_size == -1)
		return -1;

	struct sockaddr_in server_addr;
	udp_make_loopback_addr(&server_addr, port);
	const struct sockaddr *addr = (const struct sockaddr *)&server_addr;
	socklen_t addrlen = sizeof server_addr;

	if (send_pkt_start(ctx->sock_fd, transfer_id, path, file_size,
					   addr, addrlen) == -1) {
		close(fd);
		return -1;
	}
	
	if (send_file_payloads(ctx->sock_fd, transfer_id, fd, file_size,
						   addr, addrlen) == -1) {
		close(fd);
		return -1;
	}

	if (send_pkt_done(ctx->sock_fd, transfer_id, addr, addrlen) == -1) {
		close(fd);
		return -1;
    }

    int n = wait_ack(ctx, transfer_id);
    if (n == -1) {
        fprintf(stderr, "Upload unsuccessful!\n");
    } else {
        fprintf(stderr, "Upload successful!\n");
    }

	close(fd);
	return 0;
}
