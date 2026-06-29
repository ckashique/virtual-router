#include "rtable.h"

#include <stdlib.h>
#include <string.h>

#define VR_RTABLE_INITIAL_CAPACITY 8

static int vr_rtable_append(vr_rtable_t *table, const vr_route_t *route)
{
	vr_route_t *routes;

	if (table == NULL || route == NULL)
		return -1;

	if (table->count == table->capacity) {
		size_t new_cap = table->capacity == 0 ?
				 VR_RTABLE_INITIAL_CAPACITY :
				 table->capacity * 2;

		routes = realloc(table->routes, new_cap * sizeof(*routes));
		if (routes == NULL)
			return -1;

		table->routes = routes;
		table->capacity = new_cap;
	}

	table->routes[table->count++] = *route;
	return 0;
}

static const vr_route_t *pick_better_match(const vr_route_t *current,
					   const vr_route_t *candidate)
{
	if (current == NULL)
		return candidate;

	if (candidate->prefix_len > current->prefix_len)
		return candidate;

	if (candidate->prefix_len < current->prefix_len)
		return current;

	if (candidate->type == VR_ROUTE_STATIC &&
	    current->type == VR_ROUTE_CONNECTED)
		return candidate;

	return current;
}

void vr_rtable_init(vr_rtable_t *table)
{
	if (table == NULL)
		return;

	memset(table, 0, sizeof(*table));
}

void vr_rtable_free(vr_rtable_t *table)
{
	if (table == NULL)
		return;

	free(table->routes);
	vr_rtable_init(table);
}

int vr_rtable_add_connected(vr_rtable_t *table, const char *iface_name,
			    vr_ipv4_t addr, unsigned prefix_len, bool active)
{
	vr_route_t route;

	if (table == NULL)
		return -1;

	if (vr_route_make_connected(&route, iface_name, addr, prefix_len,
				    active) != 0)
		return -1;

	return vr_rtable_append(table, &route);
}

int vr_rtable_add_static(vr_rtable_t *table, const char *iface_name,
			 vr_ipv4_t prefix, unsigned prefix_len,
			 vr_ipv4_t next_hop, bool active)
{
	vr_route_t route;

	if (table == NULL)
		return -1;

	if (vr_route_make_static(&route, iface_name, prefix, prefix_len,
				 next_hop, active) != 0)
		return -1;

	return vr_rtable_append(table, &route);
}

size_t vr_rtable_count(const vr_rtable_t *table)
{
	if (table == NULL)
		return 0;

	return table->count;
}

const vr_route_t *vr_rtable_get(const vr_rtable_t *table, size_t index)
{
	if (table == NULL || index >= table->count)
		return NULL;

	return &table->routes[index];
}

const vr_route_t *vr_rtable_lpm(const vr_rtable_t *table, vr_ipv4_t dest)
{
	const vr_route_t *best = NULL;
	size_t i;

	if (table == NULL)
		return NULL;

	for (i = 0; i < table->count; i++) {
		const vr_route_t *route = &table->routes[i];

		if (!route->active || !vr_route_matches(route, dest))
			continue;

		best = pick_better_match(best, route);
	}

	return best;
}

static vr_route_t *find_connected_route(vr_rtable_t *table,
					  const char *iface_name)
{
	size_t i;

	if (table == NULL || iface_name == NULL)
		return NULL;

	for (i = 0; i < table->count; i++) {
		vr_route_t *route = &table->routes[i];

		if (route->type == VR_ROUTE_CONNECTED &&
		    strcmp(route->iface, iface_name) == 0)
			return route;
	}

	return NULL;
}

static int upsert_connected_route(vr_rtable_t *table, const vr_iface_t *iface)
{
	vr_route_t *route;

	if (table == NULL || iface == NULL)
		return -1;

	route = find_connected_route(table, iface->name);
	if (route != NULL)
		return vr_route_make_connected(route, iface->name, iface->addr,
					       iface->prefix_len, iface->up);

	return vr_rtable_add_connected(table, iface->name, iface->addr,
				       iface->prefix_len, iface->up);
}

int vr_rtable_sync_connected(vr_rtable_t *table, const vr_ifaces_t *ifaces)
{
	size_t i;

	if (table == NULL || ifaces == NULL)
		return -1;

	for (i = 0; i < vr_iface_count(ifaces); i++) {
		const vr_iface_t *iface = vr_iface_at(ifaces, i);

		if (iface == NULL)
			return -1;

		if (upsert_connected_route(table, iface) != 0)
			return -1;
	}

	return 0;
}

int vr_rtable_on_iface_change(vr_rtable_t *table, const vr_ifaces_t *ifaces,
			      const char *iface_name)
{
	const vr_iface_t *iface;

	if (table == NULL || ifaces == NULL)
		return -1;

	iface = vr_iface_get(ifaces, iface_name);
	if (iface == NULL)
		return -1;

	return upsert_connected_route(table, iface);
}
