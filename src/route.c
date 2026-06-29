#include "route.h"

#include <stddef.h>
#include <string.h>

static int validate_iface_name(const char *iface_name)
{
	size_t len;

	if (iface_name == NULL || iface_name[0] == '\0')
		return -1;

	len = strlen(iface_name);
	if (len >= VR_IFACE_NAME_MAX)
		return -1;

	return 0;
}

static int validate_prefix_len(unsigned prefix_len)
{
	if (prefix_len > 32)
		return -1;

	return 0;
}

static int copy_iface_name(vr_route_t *route, const char *iface_name)
{
	if (validate_iface_name(iface_name) != 0)
		return -1;

	strncpy(route->iface, iface_name, VR_IFACE_NAME_MAX);
	route->iface[VR_IFACE_NAME_MAX - 1] = '\0';
	return 0;
}

int vr_route_make_connected(vr_route_t *route, const char *iface_name,
			    vr_ipv4_t addr, unsigned prefix_len, bool active)
{
	if (route == NULL)
		return -1;

	if (validate_prefix_len(prefix_len) != 0)
		return -1;

	if (copy_iface_name(route, iface_name) != 0)
		return -1;

	route->type = VR_ROUTE_CONNECTED;
	route->prefix = vr_ipv4_network(addr, prefix_len);
	route->prefix_len = prefix_len;
	route->next_hop = 0;
	route->has_next_hop = false;
	route->active = active;
	return 0;
}

int vr_route_make_static(vr_route_t *route, const char *iface_name,
			 vr_ipv4_t prefix, unsigned prefix_len,
			 vr_ipv4_t next_hop, bool active)
{
	if (route == NULL)
		return -1;

	if (validate_prefix_len(prefix_len) != 0)
		return -1;

	if (copy_iface_name(route, iface_name) != 0)
		return -1;

	route->type = VR_ROUTE_STATIC;
	route->prefix = vr_ipv4_network(prefix, prefix_len);
	route->prefix_len = prefix_len;
	route->next_hop = next_hop;
	route->has_next_hop = true;
	route->active = active;
	return 0;
}

bool vr_route_matches(const vr_route_t *route, vr_ipv4_t dest)
{
	if (route == NULL)
		return false;

	return vr_ipv4_in_network(dest, route->prefix, route->prefix_len);
}
