#include "cli.h"

#include "ip_addr.h"
#include "lookup.h"

#include <string.h>

void vr_cli_show_interfaces(FILE *out, const vr_ifaces_t *ifaces)
{
	size_t i;

	if (out == NULL || ifaces == NULL)
		return;

	fprintf(out, "Interfaces (%zu):\n", vr_iface_count(ifaces));
	for (i = 0; i < vr_iface_count(ifaces); i++) {
		const vr_iface_t *iface = vr_iface_at(ifaces, i);

		if (iface == NULL)
			continue;

		fprintf(out, "  %s  " VR_IPV4_FMT "/%u  %s\n",
			iface->name, VR_IPV4_ARGS(iface->addr), iface->prefix_len,
			iface->up ? "up" : "down");
	}
}

void vr_cli_show_routes(FILE *out, const vr_rtable_t *table)
{
	size_t i;

	if (out == NULL || table == NULL)
		return;

	fprintf(out, "Routes (%zu):\n", vr_rtable_count(table));
	for (i = 0; i < vr_rtable_count(table); i++) {
		const vr_route_t *route = vr_rtable_get(table, i);

		if (route == NULL)
			continue;

		fprintf(out, "  %s  " VR_IPV4_FMT "/%u  iface=%s  %s",
			route->type == VR_ROUTE_CONNECTED ? "connected" : "static",
			VR_IPV4_ARGS(route->prefix), route->prefix_len,
			route->iface, route->active ? "active" : "inactive");

		if (route->has_next_hop)
			fprintf(out, "  via " VR_IPV4_FMT,
				VR_IPV4_ARGS(route->next_hop));

		fprintf(out, "\n");
	}
}

void vr_cli_print_help(FILE *out)
{
	if (out == NULL)
		return;

	fprintf(out, "Commands:\n");
	fprintf(out, "  show interfaces    List interfaces\n");
	fprintf(out, "  show routes        List routing table\n");
	fprintf(out, "  lookup <ip>        Longest-prefix route lookup\n");
	fprintf(out, "  replay             Replay events from JSON\n");
	fprintf(out, "  help               Show this help\n");
	fprintf(out, "  quit               Exit (q, exit)\n");
}

static void trim_newline(char *line)
{
	size_t len;

	if (line == NULL)
		return;

	len = strlen(line);
	while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r'))
		line[--len] = '\0';
}

static const char *skip_spaces(const char *str)
{
	if (str == NULL)
		return NULL;

	while (*str == ' ' || *str == '\t')
		str++;

	return str;
}

static int cmd_lookup(vr_cli_ctx *ctx, const char *ip_str)
{
	vr_ipv4_t dest;

	ip_str = skip_spaces(ip_str);

	if (ctx == NULL || ctx->table == NULL || ip_str == NULL ||
	    ip_str[0] == '\0') {
		fprintf(stderr, "Usage: lookup <ip>\n");
		return -1;
	}

	if (vr_ipv4_parse(ip_str, &dest) != 0) {
		fprintf(stderr, "Invalid destination IPv4: %s\n", ip_str);
		return -1;
	}

	vr_lookup_explain(stdout, ctx->table, dest);
	return 0;
}

static int cmd_replay(vr_cli_ctx *ctx)
{
	if (ctx == NULL || ctx->ifaces == NULL || ctx->table == NULL ||
	    ctx->events == NULL || ctx->events_path == NULL)
		return -1;

	vr_events_free(ctx->events);
	vr_events_init(ctx->events);

	if (vr_events_load(ctx->events_path, ctx->events) != 0) {
		fprintf(stderr, "Failed to load events from %s\n",
			ctx->events_path);
		return -1;
	}

	printf("Replaying %zu event(s)...\n", ctx->events->count);
	if (vr_events_replay_all(ctx->events, ctx->ifaces, ctx->table) != 0) {
		fprintf(stderr, "Failed to replay events\n");
		return -1;
	}

	return 0;
}

static int dispatch_command(vr_cli_ctx *ctx, char *line)
{
	const char *lookup_prefix = "lookup ";
	size_t lookup_prefix_len = strlen(lookup_prefix);

	if (line == NULL || line[0] == '\0')
		return 0;

	if (strcmp(line, "quit") == 0 || strcmp(line, "exit") == 0 ||
	    strcmp(line, "q") == 0)
		return 1;

	if (strcmp(line, "help") == 0) {
		vr_cli_print_help(stdout);
		return 0;
	}

	if (strcmp(line, "show interfaces") == 0) {
		vr_cli_show_interfaces(stdout, ctx->ifaces);
		return 0;
	}

	if (strcmp(line, "show routes") == 0) {
		vr_cli_show_routes(stdout, ctx->table);
		return 0;
	}

	if (strcmp(line, "replay") == 0) {
		cmd_replay(ctx);
		return 0;
	}

	if (strncmp(line, lookup_prefix, lookup_prefix_len) == 0) {
		cmd_lookup(ctx, line + lookup_prefix_len);
		return 0;
	}

	fprintf(stderr, "Unknown command: %s\n", line);
	vr_cli_print_help(stderr);
	return 0;
}

int vr_cli_run(vr_cli_ctx *ctx)
{
	char line[256];
	int rc;

	if (ctx == NULL)
		return -1;

	printf("Type 'help' for commands.\n");

	while (printf("vr> "), fflush(stdout), fgets(line, sizeof(line), stdin)) {
		trim_newline(line);
		rc = dispatch_command(ctx, line);
		if (rc == 1)
			break;
	}

	return 0;
}
