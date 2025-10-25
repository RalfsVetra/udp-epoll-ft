#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdbool.h>
#include "uthash.h"
#include "transfer.h"

struct transfer_state *g_tbl = NULL;

struct transfer_state *transfer_find(const uint8_t transfer_id[16])
{
    struct transfer_state *t = NULL;
    HASH_FIND(hh, g_tbl, transfer_id, 16, t);
    return t;
}

void transfer_add(const uint8_t transfer_id[16], int fd, uint32_t total_size,
		  const struct sockaddr *client, socklen_t client_len)
{
    struct transfer_state *t = calloc(1, sizeof *t);
    if (!t)
	return;

    memcpy(t->transfer_id, transfer_id, sizeof t->transfer_id);
    t->fd = fd;
    t->total_size = total_size;
    t->bytes_received = 0;
	
    if (client && client_len > 0) {
	memcpy(&t->client, client, client_len);
	t->client_len = client_len;
    } else {
	t->client_len = 0;
    }

    HASH_ADD(hh, g_tbl, transfer_id, 16, t);
}

void transfer_remove(struct transfer_state *t)
{
    if (!t)
	return;

    HASH_DELETE(hh, g_tbl, t);

    if (t->fd >= 0)
	close(t->fd);
    free(t);
}

void transfer_update_client(struct transfer_state *t,
			    const struct sockaddr *client, socklen_t client_len)
{
    if (!t || !client || client_len == 0)
	return;

    memcpy(&t->client, client, client_len);
    t->client_len = client_len;
}

bool transfer_write_chunk(struct transfer_state *t, uint32_t offset,
			  const uint8_t *payload, uint16_t payload_len)
{
    if (!t || t->fd < 0)
	return false;

    if (lseek(t->fd, (off_t)offset, SEEK_SET) == -1)
	return false;

    ssize_t w = write(t->fd, payload, payload_len);
    if (w != (ssize_t)payload_len)
	return false;

    t->bytes_received += payload_len;
    return true;
}

int transfer_complete(const struct transfer_state *t)
{
    if (t && (t->bytes_received == t->total_size))
	return 0;
    return -1;
}

