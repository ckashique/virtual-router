#ifndef VR_IFACE_H
#define VR_IFACE_H

#include <stdbool.h>
#include <stddef.h>

#include "ip_addr.h"

#define VR_IFACE_NAME_MAX 16

typedef struct vr_iface {
	char name[VR_IFACE_NAME_MAX];
	vr_ipv4_t addr;
	unsigned prefix_len;
	bool up;
} vr_iface_t;

typedef struct vr_ifaces {
	vr_iface_t *ifaces;
	size_t count;
	size_t capacity;
} vr_ifaces_t;

void vr_ifaces_init(vr_ifaces_t *ifaces);
void vr_ifaces_free(vr_ifaces_t *ifaces);

int vr_iface_create(vr_ifaces_t *ifaces, const char *name, vr_ipv4_t addr,
		    unsigned prefix_len, bool up);

int vr_iface_set_state(vr_ifaces_t *ifaces, const char *name, bool up);

const vr_iface_t *vr_iface_get(const vr_ifaces_t *ifaces, const char *name);

const vr_iface_t *vr_iface_at(const vr_ifaces_t *ifaces, size_t index);

size_t vr_iface_count(const vr_ifaces_t *ifaces);

#endif /* VR_IFACE_H */
