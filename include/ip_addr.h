#ifndef VR_IP_ADDR_H
#define VR_IP_ADDR_H

#include <stdbool.h>
#include <stdint.h>

/*
 * Internal IPv4 representation in host byte order.
 * Octet a.b.c.d is stored as (a << 24) | (b << 16) | (c << 8) | d.
 */
typedef uint32_t vr_ipv4_t;

#define VR_IPV4_FMT     "%u.%u.%u.%u"
#define VR_IPV4_ARGS(a) (((a) >> 24) & 0xffu), (((a) >> 16) & 0xffu), \
                        (((a) >> 8) & 0xffu), ((a) & 0xffu)

/* Parse a dotted-quad IPv4 string (e.g. "192.168.1.1"). Returns 0 on success. */
int vr_ipv4_parse(const char *str, vr_ipv4_t *out);

/* Convert prefix length (0–32) to a subnet mask. Returns 0 on success. */
int vr_ipv4_prefix_to_mask(unsigned prefix_len, vr_ipv4_t *mask_out);

/* Return the network address for addr at the given prefix length. */
vr_ipv4_t vr_ipv4_network(vr_ipv4_t addr, unsigned prefix_len);

/* Return true if addr belongs to network/prefix_len. */
bool vr_ipv4_in_network(vr_ipv4_t addr, vr_ipv4_t network, unsigned prefix_len);

#endif /* VR_IP_ADDR_H */
