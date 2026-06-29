#ifndef VR_RTABLE_H
#define VR_RTABLE_H

#include <stddef.h>

#include "iface.h"
#include "route.h"

typedef struct vr_rtable {
	vr_route_t *routes;
	size_t count;
	size_t capacity;
} vr_rtable_t;

void vr_rtable_init(vr_rtable_t *table);
void vr_rtable_free(vr_rtable_t *table);

int vr_rtable_add_connected(vr_rtable_t *table, const char *iface_name,
			    vr_ipv4_t addr, unsigned prefix_len, bool active);

int vr_rtable_add_static(vr_rtable_t *table, const char *iface_name,
			 vr_ipv4_t prefix, unsigned prefix_len,
			 vr_ipv4_t next_hop, bool active);

size_t vr_rtable_count(const vr_rtable_t *table);
const vr_route_t *vr_rtable_get(const vr_rtable_t *table, size_t index);

const vr_route_t *vr_rtable_lpm(const vr_rtable_t *table, vr_ipv4_t dest);

int vr_rtable_sync_connected(vr_rtable_t *table, const vr_ifaces_t *ifaces);

int vr_rtable_on_iface_change(vr_rtable_t *table, const vr_ifaces_t *ifaces,
			      const char *iface_name);

#endif /* VR_RTABLE_H */
