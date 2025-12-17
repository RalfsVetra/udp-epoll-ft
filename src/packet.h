#ifndef PACKET_H
#define PACKET_H

#include <stdint.h>
#include <sys/socket.h>

enum {
    PKT_START,
    PKT_PAYLOAD,
    PKT_DONE,
    PKT_OK,
    PKT_ERROR
};

#pragma pack(push, 1)
struct pkt_start {
    uint8_t  type;
    uint32_t total_size;
    uint16_t file_name_len;
    uint8_t  transfer_id[16];
    char     file_name[];
};

struct pkt_payload {
    uint8_t  type;
    uint32_t offset;
    uint16_t payload_len;
    uint8_t  transfer_id[16];
    uint8_t  payload[];
};

struct pkt_done {
    uint8_t type;
    uint8_t transfer_id[16];
};

struct pkt_ack {
    uint8_t type;
    uint8_t transfer_id[16];
};
#pragma pack(pop)

void parse_pkt_start(const struct pkt_start *buf, size_t len,
					 const struct sockaddr *client, socklen_t client_len);
void parse_pkt_payload(const struct pkt_payload *buf, size_t len,
					   const struct sockaddr *client, socklen_t client_len);
int parse_pkt_done(const struct pkt_done *buf, size_t len);

int build_pkt_start(uint8_t *dst, size_t dst_cap,
					const uint8_t transfer_id[16], const char *path, int32_t file_size);
int build_pkt_payload(uint8_t *dst, size_t dst_cap, const uint8_t transfer_id[16],
					  uint32_t offset, const uint8_t *data, uint16_t len);
int build_pkt_done(uint8_t *dst, const uint8_t transfer_id[16]);

#endif /* PACKET_H */
