#ifndef VR_LOOKUP_H
#define VR_LOOKUP_H

#include <stdio.h>

#include "ip_addr.h"
#include "route.h"
#include "rtable.h"

const vr_route_t *vr_lookup_route(const vr_rtable_t *table, vr_ipv4_t dest);

void vr_lookup_explain(FILE *out, const vr_rtable_t *table, vr_ipv4_t dest);

#endif /* VR_LOOKUP_H */
