#include "json_input.h"

#include "json_util.h"
#include "ip_addr.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

static bool parse_iface_state(const char *state)
{
	if (state == NULL)
		return false;

	if (strcmp(state, "up") == 0)
		return true;
	if (strcmp(state, "down") == 0)
		return false;

	return false;
}

static bool iface_is_up(const vr_ifaces_t *ifaces, const char *iface_name)
{
	const vr_iface_t *iface = vr_iface_get(ifaces, iface_name);

	if (iface == NULL)
		return false;

	return iface->up;
}

int vr_json_load_interfaces(const char *path, vr_ifaces_t *ifaces)
{
	char *json;
	size_t json_len;
	const char *end;
	const char *array;
	const char *p;
	const char *obj_start;
	const char *obj_end;
	char name[VR_IFACE_NAME_MAX];
	char address[32];
	char state[16];
	unsigned prefix_len;
	vr_ipv4_t addr;

	if (path == NULL || ifaces == NULL)
		return -1;

	json = vr_json_read_file(path, &json_len);
	if (json == NULL)
		return -1;

	end = json + json_len;
	array = vr_json_find_key_array(json, end, "interfaces");
	if (array == NULL) {
		free(json);
		return -1;
	}

	p = array;
	while ((obj_start = vr_json_array_next_object(&p, end, &obj_end))) {
		if (vr_json_object_string(obj_start, obj_end, "name", name,
					  sizeof(name)) != 0 ||
		    vr_json_object_string(obj_start, obj_end, "address", address,
					  sizeof(address)) != 0 ||
		    vr_json_object_uint(obj_start, obj_end, "prefix_length",
					&prefix_len, 32) != 0 ||
		    vr_json_object_string(obj_start, obj_end, "state", state,
					  sizeof(state)) != 0) {
			free(json);
			return -1;
		}

		if (vr_ipv4_parse(address, &addr) != 0 ||
		    vr_iface_create(ifaces, name, addr, prefix_len,
				    parse_iface_state(state)) != 0) {
			free(json);
			return -1;
		}
	}

	if (!vr_json_at_array_end(p, end)) {
		free(json);
		return -1;
	}

	free(json);
	return 0;
}

int vr_json_load_static_routes(const char *path, const vr_ifaces_t *ifaces,
			       vr_rtable_t *table)
{
	char *json;
	size_t json_len;
	const char *end;
	const char *array;
	const char *p;
	const char *obj_start;
	const char *obj_end;
	char prefix_str[32];
	char next_hop_str[32];
	char iface_name[VR_IFACE_NAME_MAX];
	unsigned prefix_len;
	vr_ipv4_t prefix;
	vr_ipv4_t next_hop;
	bool active;

	if (path == NULL || ifaces == NULL || table == NULL)
		return -1;

	json = vr_json_read_file(path, &json_len);
	if (json == NULL)
		return -1;

	end = json + json_len;
	array = vr_json_find_key_array(json, end, "routes");
	if (array == NULL) {
		free(json);
		return -1;
	}

	p = array;
	while ((obj_start = vr_json_array_next_object(&p, end, &obj_end))) {
		if (vr_json_object_string(obj_start, obj_end, "prefix", prefix_str,
					  sizeof(prefix_str)) != 0 ||
		    vr_json_object_uint(obj_start, obj_end, "prefix_length",
					&prefix_len, 32) != 0 ||
		    vr_json_object_string(obj_start, obj_end, "next_hop",
					  next_hop_str,
					  sizeof(next_hop_str)) != 0 ||
		    vr_json_object_string(obj_start, obj_end, "interface",
					  iface_name,
					  sizeof(iface_name)) != 0) {
			free(json);
			return -1;
		}

		if (vr_ipv4_parse(prefix_str, &prefix) != 0 ||
		    vr_ipv4_parse(next_hop_str, &next_hop) != 0) {
			free(json);
			return -1;
		}

		active = iface_is_up(ifaces, iface_name);
		if (vr_rtable_add_static(table, iface_name, prefix, prefix_len,
					 next_hop, active) != 0) {
			free(json);
			return -1;
		}
	}

	if (!vr_json_at_array_end(p, end)) {
		free(json);
		return -1;
	}

	free(json);
	return 0;
}
