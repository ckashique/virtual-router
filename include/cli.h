#ifndef VR_CLI_H
#define VR_CLI_H

#include <stdio.h>

#include "events.h"
#include "iface.h"
#include "rtable.h"

typedef struct vr_cli_ctx {
	vr_ifaces_t *ifaces;
	vr_rtable_t *table;
	vr_events_t *events;
	const char *events_path;
} vr_cli_ctx;

void vr_cli_show_interfaces(FILE *out, const vr_ifaces_t *ifaces);

void vr_cli_show_routes(FILE *out, const vr_rtable_t *table);

void vr_cli_print_help(FILE *out);

int vr_cli_run(vr_cli_ctx *ctx);

#endif /* VR_CLI_H */
