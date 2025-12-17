#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <limits.h>
#include "packet.h"
#include "transfer.h"
#include "util.h"

#define UPLOAD_DIR "uploads/"

void parse_pkt_start(const struct pkt_start *s, size_t len,
					 const struct sockaddr *client, socklen_t client_len)
{
    if (len < sizeof(*s))
		return;
	
    uint32_t total_size    = ntohl(s->total_size);
    uint16_t file_name_len = ntohs(s->file_name_len);

    size_t need = sizeof(*s) + (size_t)file_name_len;
    if (len < need)
		return;

    const char *file_name = (const char *)((const uint8_t *)s + sizeof(*s));
    const char *prefix = UPLOAD_DIR;
    char upload_path[PATH_MAX];

    size_t remain = sizeof(upload_path) - strlen(prefix) - 1;
    size_t take = file_name_len < remain ? file_name_len : remain;

    snprintf(upload_path, sizeof upload_path, "%s%.*s",
             prefix, (int)take, file_name);
    
    printf("Processing incoming packets from %.*s\n",
           (int)file_name_len, file_name);
    
    int fd = open(upload_path, O_CREAT|O_RDWR|O_TRUNC, 0644);
    if (fd == -1)
		return;

    // This will almost never happen, but might
    struct transfer_state *t = transfer_find(s->transfer_id);
    if (t)
		transfer_remove(t);

    transfer_add(s->transfer_id, fd, total_size, client, client_len);
}

void parse_pkt_payload(const struct pkt_payload *p, size_t len,
					   const struct sockaddr *client, socklen_t client_len)
{
    if (len < sizeof(*p))
		return;
	
    uint32_t offset      = ntohl(p->offset);
    uint16_t payload_len = ntohs(p->payload_len);

    size_t need = sizeof(*p) + (size_t)payload_len;
    if (len < need)
		return;

    const uint8_t *payload = (const uint8_t *)((const uint8_t *)p + sizeof(*p));
	
    struct transfer_state *t = transfer_find(p->transfer_id);
    if (!t)
		return;

    transfer_update_client(t, client, client_len);

    if (offset != t->bytes_received)
		return;

    if (!transfer_write_chunk(t, offset, payload, payload_len))
		return;
}

int parse_pkt_done(const struct pkt_done *d, size_t len)
{
    if (len < sizeof(*d))
		return -1;

    struct transfer_state *t = transfer_find(d->transfer_id);
    if (!t)
		return -1;

    int done = transfer_complete(t);
    
    fsync(t->fd);
    transfer_remove(t);
    return done;
}

int parse_pkt_ack(const struct pkt_ack *a, size_t len)
{
	if (len < sizeof(*a))
		return -1;

	if (a->type != PKT_OK)
		return -1;
	return 0;
}

int build_pkt_start(uint8_t *dst, size_t dst_cap,
					const uint8_t transfer_id[16], const char *path, int32_t file_size)
{
	const char *name = get_file_name(path);
	size_t name_len = strlen(name);
	if (name_len > UINT16_MAX)
		return -1;

	struct pkt_start s = {
		.type = PKT_START,
		.total_size = htonl((uint32_t)file_size),
		.file_name_len = htons((uint16_t)name_len)
	};
	memcpy(s.transfer_id, transfer_id, 16);

	size_t need = sizeof s + name_len;
	if (need > dst_cap)
		return -1;

	memcpy(dst, &s, sizeof s);
	memcpy(dst + sizeof s, name, name_len);
	return (int)need;
}

int build_pkt_payload(uint8_t *dst, size_t dst_cap, const uint8_t transfer_id[16],
					  uint32_t offset, const uint8_t *data, uint16_t len)
{
	struct pkt_payload p = {
		.type = PKT_PAYLOAD,
		.offset = htonl(offset),
		.payload_len = htons(len)
	};
    memcpy(p.transfer_id, transfer_id, 16);

	size_t need = sizeof p + len;
	if (need > dst_cap)
		return -1;

    memcpy(dst, &p, sizeof p);
    memcpy(dst + sizeof p, data, len);
    return (int)need;
}

int build_pkt_done(uint8_t *dst, const uint8_t transfer_id[16])
{
	struct pkt_done d = {
		.type = PKT_DONE
	};
	memcpy(d.transfer_id, transfer_id, 16);
	memcpy(dst, &d, sizeof d);
	return (int)sizeof d;
}
