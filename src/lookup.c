#include "lookup.h"

const vr_route_t *vr_lookup_route(const vr_rtable_t *table, vr_ipv4_t dest)
{
	return vr_rtable_lpm(table, dest);
}

static void print_route(FILE *out, const vr_route_t *route)
{
	if (out == NULL)
		return;

	if (route == NULL) {
		fprintf(out, "no matching route\n");
		return;
	}

	fprintf(out, "%s  " VR_IPV4_FMT "/%u  iface=%s  %s",
		route->type == VR_ROUTE_CONNECTED ? "connected" : "static",
		VR_IPV4_ARGS(route->prefix), route->prefix_len,
		route->iface, route->active ? "active" : "inactive");

	if (route->has_next_hop)
		fprintf(out, "  via " VR_IPV4_FMT, VR_IPV4_ARGS(route->next_hop));

	fprintf(out, "\n");
}

void vr_lookup_explain(FILE *out, const vr_rtable_t *table, vr_ipv4_t dest)
{
	const vr_route_t *selected = vr_lookup_route(table, dest);
	size_t i;

	if (out == NULL || table == NULL)
		return;

	fprintf(out, "Route lookup for " VR_IPV4_FMT ":\n", VR_IPV4_ARGS(dest));

	for (i = 0; i < vr_rtable_count(table); i++) {
		const vr_route_t *route = vr_rtable_get(table, i);
		bool matched;

		if (route == NULL || !route->active)
			continue;

		matched = vr_route_matches(route, dest);
		fprintf(out, "  %s  " VR_IPV4_FMT "/%u  prefix_len=%u  matched=%s\n",
			route->type == VR_ROUTE_CONNECTED ? "connected" :
							      "static",
			VR_IPV4_ARGS(route->prefix), route->prefix_len,
			route->prefix_len, matched ? "yes" : "no");
	}

	if (selected == NULL) {
		fprintf(out,
			"Result: no active route matched the destination.\n");
		return;
	}

	fprintf(out, "Selected route:\n  ");
	print_route(out, selected);
	fprintf(out,
		"Reason: longest prefix match (/%u is the longest matching prefix among active routes).\n",
		selected->prefix_len);
}
