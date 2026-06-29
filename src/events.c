#include "events.h"

#include "json_util.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VR_EVENTS_INITIAL_CAPACITY 8

static int append_event(vr_events_t *events, const vr_event_t *event)
{
	vr_event_t *entries;

	if (events == NULL || event == NULL)
		return -1;

	if (events->count == events->capacity) {
		size_t new_cap = events->capacity == 0 ?
				 VR_EVENTS_INITIAL_CAPACITY :
				 events->capacity * 2;

		entries = realloc(events->events, new_cap * sizeof(*entries));
		if (entries == NULL)
			return -1;

		events->events = entries;
		events->capacity = new_cap;
	}

	events->events[events->count++] = *event;
	return 0;
}

static int compare_events(const void *a, const void *b)
{
	const vr_event_t *left = a;
	const vr_event_t *right = b;

	if (left->seq < right->seq)
		return -1;
	if (left->seq > right->seq)
		return 1;

	return 0;
}

static int parse_event_type(const char *type_str, vr_event_type *type_out)
{
	if (type_str == NULL || type_out == NULL)
		return -1;

	if (strcmp(type_str, "iface_up") == 0) {
		*type_out = VR_EVENT_IFACE_UP;
		return 0;
	}

	if (strcmp(type_str, "iface_down") == 0) {
		*type_out = VR_EVENT_IFACE_DOWN;
		return 0;
	}

	return -1;
}

static const char *event_type_string(vr_event_type type)
{
	return type == VR_EVENT_IFACE_UP ? "iface_up" : "iface_down";
}

static int apply_event(const vr_event_t *event, vr_ifaces_t *ifaces,
		       vr_rtable_t *table)
{
	bool up;

	if (event == NULL || ifaces == NULL || table == NULL)
		return -1;

	up = event->type == VR_EVENT_IFACE_UP;

	printf("event seq=%u type=%s interface=%s\n", event->seq,
	       event_type_string(event->type), event->interface);

	if (vr_iface_set_state(ifaces, event->interface, up) != 0)
		return -1;

	if (vr_rtable_on_iface_change(table, ifaces, event->interface) != 0)
		return -1;

	return 0;
}

static int replay_next(vr_events_t *events, vr_ifaces_t *ifaces,
		       vr_rtable_t *table)
{
	if (events == NULL || ifaces == NULL || table == NULL)
		return -1;

	if (events->next_index >= events->count)
		return 0;

	if (apply_event(&events->events[events->next_index], ifaces, table) != 0)
		return -1;

	events->next_index++;
	return 1;
}

void vr_events_init(vr_events_t *events)
{
	if (events == NULL)
		return;

	memset(events, 0, sizeof(*events));
}

void vr_events_free(vr_events_t *events)
{
	if (events == NULL)
		return;

	free(events->events);
	vr_events_init(events);
}

int vr_events_load(const char *path, vr_events_t *events)
{
	char *json;
	size_t json_len;
	const char *end;
	const char *array;
	const char *p;
	const char *obj_start;
	const char *obj_end;
	vr_event_t event;
	char type_str[32];
	char iface_name[VR_IFACE_NAME_MAX];

	if (path == NULL || events == NULL)
		return -1;

	json = vr_json_read_file(path, &json_len);
	if (json == NULL)
		return -1;

	end = json + json_len;
	array = vr_json_find_key_array(json, end, "events");
	if (array == NULL) {
		free(json);
		return -1;
	}

	p = array;
	while ((obj_start = vr_json_array_next_object(&p, end, &obj_end))) {
		memset(&event, 0, sizeof(event));
		if (vr_json_object_uint(obj_start, obj_end, "seq", &event.seq,
					UINT_MAX) != 0 ||
		    vr_json_object_string(obj_start, obj_end, "type", type_str,
					  sizeof(type_str)) != 0 ||
		    vr_json_object_string(obj_start, obj_end, "interface",
					  iface_name,
					  sizeof(iface_name)) != 0 ||
		    parse_event_type(type_str, &event.type) != 0) {
			free(json);
			return -1;
		}

		strncpy(event.interface, iface_name, VR_IFACE_NAME_MAX);
		event.interface[VR_IFACE_NAME_MAX - 1] = '\0';

		if (append_event(events, &event) != 0) {
			free(json);
			return -1;
		}
	}

	if (!vr_json_at_array_end(p, end)) {
		free(json);
		return -1;
	}

	free(json);

	if (events->count > 1)
		qsort(events->events, events->count, sizeof(*events->events),
		      compare_events);

	events->next_index = 0;
	return 0;
}

int vr_events_replay_all(vr_events_t *events, vr_ifaces_t *ifaces,
			 vr_rtable_t *table)
{
	int rc;

	while ((rc = replay_next(events, ifaces, table)) == 1)
		;

	return rc;
}
