#ifndef VR_ROUTE_H
#define VR_ROUTE_H

#include <stdbool.h>

#include "ip_addr.h"

#define VR_IFACE_NAME_MAX 16

typedef enum vr_route_type {
	VR_ROUTE_CONNECTED,
	VR_ROUTE_STATIC,
} vr_route_type;

typedef struct vr_route {
	vr_route_type type;
	vr_ipv4_t prefix;
	unsigned prefix_len;
	vr_ipv4_t next_hop;
	bool has_next_hop;
	char iface[VR_IFACE_NAME_MAX];
	bool active;
} vr_route_t;

int vr_route_make_connected(vr_route_t *route, const char *iface_name,
			    vr_ipv4_t addr, unsigned prefix_len, bool active);

int vr_route_make_static(vr_route_t *route, const char *iface_name,
			 vr_ipv4_t prefix, unsigned prefix_len,
			 vr_ipv4_t next_hop, bool active);

bool vr_route_matches(const vr_route_t *route, vr_ipv4_t dest);

#endif /* VR_ROUTE_H */
