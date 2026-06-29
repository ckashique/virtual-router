#ifndef VR_JSON_INPUT_H
#define VR_JSON_INPUT_H

#include "iface.h"
#include "rtable.h"

int vr_json_load_interfaces(const char *path, vr_ifaces_t *ifaces);

int vr_json_load_static_routes(const char *path, const vr_ifaces_t *ifaces,
			       vr_rtable_t *table);

#endif /* VR_JSON_INPUT_H */
