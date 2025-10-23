#ifndef _PACKET_H
#define _PACKET_H

#include <stdint.h>
#include <sys/socket.h>

enum {
	PKT_START,
	PKT_PAYLOAD,
	PKT_DONE,
	PKT_OK,
	PKT_ERROR
};

#pragma pack(push,1)
struct pkt_start {
	uint8_t  type;
	uint8_t  transfer_id[16];
	uint64_t total_size;
	uint16_t file_name_len;
	char     file_name[];
};

struct pkt_payload {
	uint8_t  type;
	uint8_t  transfer_id[16];
	uint64_t offset;
	uint16_t payload_len;
	uint8_t  payload[];
};

struct pkt_done {
	uint8_t type;
	uint8_t transfer_id[16];
};

struct pkt_ok {
	uint8_t type;
	uint8_t transfer_id[16];
};

struct pkt_error {
	uint8_t type;
	uint8_t transfer_id[16];
};
#pragma pack(pop)

void handle_start(const struct pkt_start *buf, size_t len,
		const struct sockaddr *client, socklen_t client_len);
void handle_payload(const struct pkt_payload *buf, size_t len,
		const struct sockaddr *client, socklen_t client_len);
void handle_done(const struct pkt_done *buf, size_t len,
		const struct sockaddr *client, socklen_t client_len);

#endif /* _PACKET_H */
