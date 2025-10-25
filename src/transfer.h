#ifndef _TRANSFER_H
#define _TRANSFER_H

#include <sys/socket.h>
#include <stdint.h>
#include <stdbool.h>
#include "uthash.h"

struct transfer_state {
    uint8_t                 transfer_id[16];
    int                     fd;
    uint32_t                total_size;
    uint32_t                bytes_received;
    struct sockaddr_storage client;
    socklen_t               client_len;
    UT_hash_handle          hh;
};

struct transfer_state *transfer_find(const uint8_t transfer_id[16]);
void transfer_add(const uint8_t transfer_id[16], int fd, uint32_t total_size,
				  const struct sockaddr *client, socklen_t client_len);
void transfer_remove(struct transfer_state *t);

void transfer_update_client(struct transfer_state *t,
							const struct sockaddr *client, socklen_t client_len);
bool transfer_write_chunk(struct transfer_state *t, uint32_t offset,
						  const uint8_t *payload, uint16_t payload_len);
int transfer_complete(const struct transfer_state *t);
void transfer_ack(const struct transfer_state *t, const struct sockaddr *client,
				  socklen_t client_len);

#endif /* _TRANSFER_H */
