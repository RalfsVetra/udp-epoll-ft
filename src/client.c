#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "client.h"
#include "context.h"
#include "packet.h"
#include "net.h"

static const char *basename_const(const char *path) {
	if (!path)
		return "";
	
    const char *slash = strrchr(path, '/');
    return slash ? slash + 1 : path;
}

static int send_all(int sock_fd, const void *buf, size_t len) {
    for (;;) {
        ssize_t n = send(sock_fd, buf, len, 0);
        if (n == (ssize_t)len)
			return 0;
		
        if (n == -1 && (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK))
            continue;
		
        return -1;
    }
}

static int send_start(int sock_fd, const uint8_t transfer_id[16],
					  const char *path, uint32_t total_size)
{
	const char *name = basename_const(path);
	size_t name_len = strlen(name);
	if (name_len > UINT16_MAX)
		return -1;

	struct pkt_start s;
	s.type = PKT_START;
	memcpy(s.transfer_id, transfer_id, 16);
	s.total_size    = htonl(total_size);
	s.file_name_len = htons((uint16_t)name_len);

	uint8_t buf[sizeof s + UINT16_MAX];
	memcpy(buf, &s, sizeof s);
	memcpy(buf + sizeof s, name, name_len);
	return send_all(sock_fd, buf, sizeof s + name_len);
}

static int send_payload(int sock_fd, const uint8_t transfer_id[16],
						uint32_t offset, const uint8_t *data, uint16_t len)
{
	struct pkt_payload p;
    p.type = PKT_PAYLOAD;
    memcpy(p.transfer_id, transfer_id, 16);
    p.offset      = htonl(offset);
    p.payload_len = htons(len);

    uint8_t buf[sizeof p + ONE_KB];
    memcpy(buf, &p, sizeof p);
    memcpy(buf + sizeof p, data, len);
    return send_all(sock_fd, buf, sizeof p + len);
}

static int send_done(int sock_fd, const uint8_t transfer_id[16])
{
	struct pkt_done d;
    d.type = PKT_DONE;
    memcpy(d.transfer_id, transfer_id, 16);
    return send_all(sock_fd, &d, sizeof d);
}

static int send_packets(int sock_fd, const uint8_t transfer_id[16], int fd)
{
	uint8_t buf[ONE_KB];
	uint32_t offset = 0;

	for (;;) {
		ssize_t n = read(fd, buf, sizeof buf);

		if (n > 0) {
			if (send_payload(sock_fd, transfer_id, offset, buf, (uint16_t)n) == -1) {
				return -1;
			}

			offset += (uint32_t)n;
		}

		if (n == 0)
			break;

		if (n == -1) {
			if (errno == EINTR)
				continue;

			return -1;
		}
	}
	return 0;
}

static int client_connect(int sock_fd, int port)
{
	struct sockaddr_in addr;
	make_loopback_addr(&addr, port);

	if (connect(sock_fd, (struct sockaddr *)&addr, sizeof addr) == -1)
        if (errno != EISCONN)
			return -1;
	return 0;
}

static int get_file_size(const char *path, uint32_t *out_size)
{
	struct stat st;
	if (stat(path, &st) == -1)
		return -1;

	*out_size = (uint32_t)st.st_size;
	return 0;
}

int client_start(struct context *ctx, const char *path, int port)
{
	if (client_connect(ctx->sock_fd, port) == -1)
		return -1;

	uint32_t file_size;
	if (get_file_size(path, &file_size) == -1)
		return -1;

	uint8_t transfer_id[16] = {0};
	arc4random_buf(transfer_id, sizeof transfer_id);

	int fd = open(path, O_RDONLY);
	if (fd == -1)
		return -1;

	if (send_start(ctx->sock_fd, transfer_id, path, file_size) == -1) {
		close(fd);
		return -1;
	}
	
	if (send_packets(ctx->sock_fd, transfer_id, fd) == -1) {
		close(fd);
		return -1;
	}

	if (send_done(ctx->sock_fd, transfer_id) == -1) {
		close(fd);
		return -1;
    }

	close(fd);
	return 0;
}

