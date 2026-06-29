#include "ip_addr.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stddef.h>

int vr_ipv4_parse(const char *str, vr_ipv4_t *out)
{
	struct in_addr in;

	if (str == NULL || out == NULL)
		return -1;

	if (inet_pton(AF_INET, str, &in) != 1)
		return -1;

	*out = ntohl(in.s_addr);
	return 0;
}

int vr_ipv4_prefix_to_mask(unsigned prefix_len, vr_ipv4_t *mask_out)
{
	if (mask_out == NULL)
		return -1;

	if (prefix_len > 32)
		return -1;

	if (prefix_len == 0) {
		*mask_out = 0;
		return 0;
	}

	if (prefix_len == 32) {
		*mask_out = 0xffffffffu;
		return 0;
	}

	*mask_out = (~0u << (32 - prefix_len));
	return 0;
}

vr_ipv4_t vr_ipv4_network(vr_ipv4_t addr, unsigned prefix_len)
{
	vr_ipv4_t mask;

	if (vr_ipv4_prefix_to_mask(prefix_len, &mask) != 0)
		return 0;

	return addr & mask;
}

bool vr_ipv4_in_network(vr_ipv4_t addr, vr_ipv4_t network, unsigned prefix_len)
{
	vr_ipv4_t mask;

	if (vr_ipv4_prefix_to_mask(prefix_len, &mask) != 0)
		return false;

	return (addr & mask) == (network & mask);
}
