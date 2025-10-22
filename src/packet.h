#ifndef _PACKET_H
#define _PACKET_H

enum {
	PKT_START,
	PKT_PAYLOAD,
	PKT_DONE
};

struct pkt_start {
	uint8_t type;
};

struct pkt_payload {
	uint8_t type;
};

struct pkt_done {
	uint8_t type;
};

#endif /* _PACKET_H */
