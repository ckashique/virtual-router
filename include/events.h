#ifndef VR_EVENTS_H
#define VR_EVENTS_H

#include <stddef.h>

#include "iface.h"
#include "rtable.h"

typedef enum vr_event_type {
	VR_EVENT_IFACE_UP,
	VR_EVENT_IFACE_DOWN,
} vr_event_type;

typedef struct vr_event {
	unsigned seq;
	vr_event_type type;
	char interface[VR_IFACE_NAME_MAX];
} vr_event_t;

typedef struct vr_events {
	vr_event_t *events;
	size_t count;
	size_t capacity;
	size_t next_index;
} vr_events_t;

void vr_events_init(vr_events_t *events);
void vr_events_free(vr_events_t *events);

int vr_events_load(const char *path, vr_events_t *events);

int vr_events_replay_all(vr_events_t *events, vr_ifaces_t *ifaces,
			 vr_rtable_t *table);

#endif /* VR_EVENTS_H */
