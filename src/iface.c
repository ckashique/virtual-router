#include "iface.h"

#include <stdlib.h>
#include <string.h>

#define VR_IFACES_INITIAL_CAPACITY 4

static int validate_name(const char *name)
{
	size_t len;

	if (name == NULL || name[0] == '\0')
		return -1;

	len = strlen(name);
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

static const vr_iface_t *find_iface(const vr_ifaces_t *ifaces, const char *name)
{
	size_t i;

	if (ifaces == NULL || name == NULL)
		return NULL;

	for (i = 0; i < ifaces->count; i++) {
		if (strcmp(ifaces->ifaces[i].name, name) == 0)
			return &ifaces->ifaces[i];
	}

	return NULL;
}

static int append_iface(vr_ifaces_t *ifaces, const vr_iface_t *iface)
{
	vr_iface_t *entries;

	if (ifaces == NULL || iface == NULL)
		return -1;

	if (ifaces->count == ifaces->capacity) {
		size_t new_cap = ifaces->capacity == 0 ?
				 VR_IFACES_INITIAL_CAPACITY :
				 ifaces->capacity * 2;

		entries = realloc(ifaces->ifaces, new_cap * sizeof(*entries));
		if (entries == NULL)
			return -1;

		ifaces->ifaces = entries;
		ifaces->capacity = new_cap;
	}

	ifaces->ifaces[ifaces->count++] = *iface;
	return 0;
}

void vr_ifaces_init(vr_ifaces_t *ifaces)
{
	if (ifaces == NULL)
		return;

	memset(ifaces, 0, sizeof(*ifaces));
}

void vr_ifaces_free(vr_ifaces_t *ifaces)
{
	if (ifaces == NULL)
		return;

	free(ifaces->ifaces);
	vr_ifaces_init(ifaces);
}

int vr_iface_create(vr_ifaces_t *ifaces, const char *name, vr_ipv4_t addr,
		    unsigned prefix_len, bool up)
{
	vr_iface_t iface;

	if (ifaces == NULL)
		return -1;

	if (validate_name(name) != 0 || validate_prefix_len(prefix_len) != 0)
		return -1;

	if (find_iface(ifaces, name) != NULL)
		return -1;

	memset(&iface, 0, sizeof(iface));
	strncpy(iface.name, name, VR_IFACE_NAME_MAX);
	iface.name[VR_IFACE_NAME_MAX - 1] = '\0';
	iface.addr = addr;
	iface.prefix_len = prefix_len;
	iface.up = up;

	return append_iface(ifaces, &iface);
}

int vr_iface_set_state(vr_ifaces_t *ifaces, const char *name, bool up)
{
	vr_iface_t *iface;

	if (ifaces == NULL || validate_name(name) != 0)
		return -1;

	iface = (vr_iface_t *)find_iface(ifaces, name);
	if (iface == NULL)
		return -1;

	iface->up = up;
	return 0;
}

const vr_iface_t *vr_iface_get(const vr_ifaces_t *ifaces, const char *name)
{
	if (ifaces == NULL || validate_name(name) != 0)
		return NULL;

	return find_iface(ifaces, name);
}

const vr_iface_t *vr_iface_at(const vr_ifaces_t *ifaces, size_t index)
{
	if (ifaces == NULL || index >= ifaces->count)
		return NULL;

	return &ifaces->ifaces[index];
}

size_t vr_iface_count(const vr_ifaces_t *ifaces)
{
	if (ifaces == NULL)
		return 0;

	return ifaces->count;
}
