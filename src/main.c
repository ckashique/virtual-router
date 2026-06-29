#include "cli.h"
#include "events.h"
#include "iface.h"
#include "json_input.h"
#include "ip_addr.h"
#include "lookup.h"
#include "rtable.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define DEFAULT_IFACES_PATH "data/interfaces.json"
#define DEFAULT_ROUTES_PATH "data/static_routes.json"
#define DEFAULT_EVENTS_PATH "data/events.json"

static void print_usage(const char *prog)
{
	fprintf(stderr,
		"Usage: %s [-i interfaces.json] [-r static_routes.json] [-e events.json] [-p] [-d IPv4]\n",
		prog);
}

int main(int argc, char *argv[])
{
	const char *ifaces_path = DEFAULT_IFACES_PATH;
	const char *routes_path = DEFAULT_ROUTES_PATH;
	const char *events_path = DEFAULT_EVENTS_PATH;
	vr_ifaces_t ifaces;
	vr_rtable_t table;
	vr_events_t events;
	vr_cli_ctx cli;
	vr_ipv4_t lookup_dest;
	bool do_lookup = false;
	bool do_print = false;
	int i;

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-i") == 0 && i + 1 < argc) {
			ifaces_path = argv[++i];
		} else if (strcmp(argv[i], "-r") == 0 && i + 1 < argc) {
			routes_path = argv[++i];
		} else if (strcmp(argv[i], "-e") == 0 && i + 1 < argc) {
			events_path = argv[++i];
		} else if (strcmp(argv[i], "-d") == 0 && i + 1 < argc) {
			if (vr_ipv4_parse(argv[++i], &lookup_dest) != 0) {
				fprintf(stderr, "Invalid destination IPv4: %s\n",
					argv[i]);
				return 1;
			}
			do_lookup = true;
		} else if (strcmp(argv[i], "-p") == 0) {
			do_print = true;
		} else if (strcmp(argv[i], "-h") == 0 ||
			   strcmp(argv[i], "--help") == 0) {
			print_usage(argv[0]);
			return 0;
		} else {
			fprintf(stderr, "Unknown argument: %s\n", argv[i]);
			print_usage(argv[0]);
			return 1;
		}
	}

	vr_ifaces_init(&ifaces);
	vr_rtable_init(&table);
	vr_events_init(&events);

	if (vr_json_load_interfaces(ifaces_path, &ifaces) != 0) {
		fprintf(stderr, "Failed to load interfaces from %s\n",
			ifaces_path);
		vr_rtable_free(&table);
		vr_ifaces_free(&ifaces);
		return 1;
	}

	if (vr_rtable_sync_connected(&table, &ifaces) != 0) {
		fprintf(stderr, "Failed to build connected routes\n");
		vr_rtable_free(&table);
		vr_ifaces_free(&ifaces);
		return 1;
	}

	if (vr_json_load_static_routes(routes_path, &ifaces, &table) != 0) {
		fprintf(stderr, "Failed to load static routes from %s\n",
			routes_path);
		vr_rtable_free(&table);
		vr_ifaces_free(&ifaces);
		return 1;
	}

	if (!do_print) {
		vr_cli_show_interfaces(stdout, &ifaces);
		vr_cli_show_routes(stdout, &table);
	}

	if (vr_events_load(events_path, &events) != 0) {
		fprintf(stderr, "Failed to load events from %s\n", events_path);
		vr_events_free(&events);
		vr_rtable_free(&table);
		vr_ifaces_free(&ifaces);
		return 1;
	}

	printf("Replaying %zu event(s)...\n", events.count);
	if (vr_events_replay_all(&events, &ifaces, &table) != 0) {
		fprintf(stderr, "Failed to replay events\n");
		vr_events_free(&events);
		vr_rtable_free(&table);
		vr_ifaces_free(&ifaces);
		return 1;
	}

	vr_cli_show_interfaces(stdout, &ifaces);
	vr_cli_show_routes(stdout, &table);

	if (do_lookup)
		vr_lookup_explain(stdout, &table, lookup_dest);

	cli.ifaces = &ifaces;
	cli.table = &table;
	cli.events = &events;
	cli.events_path = events_path;
	vr_cli_run(&cli);

	vr_events_free(&events);
	vr_rtable_free(&table);
	vr_ifaces_free(&ifaces);
	return 0;
}
