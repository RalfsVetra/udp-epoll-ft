#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <limits.h>
#include "packet.h"
#include "transfer.h"

void handle_start(const struct pkt_start *s, size_t len,
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

	char fname[PATH_MAX];
	memcpy(fname, file_name, file_name_len);
	fname[file_name_len] = '\0';

    int fd = open(fname, O_CREAT|O_RDWR|O_TRUNC, 0644);
    if (fd == -1)
		return;

    // This will almost never happen, but might
    struct transfer_state *t = transfer_find(s->transfer_id);
    if (t)
		transfer_remove(t);

    transfer_add(s->transfer_id, fd, total_size, client, client_len);
}

void handle_payload(const struct pkt_payload *p, size_t len,
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

int handle_done(const struct pkt_done *d, size_t len)
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

