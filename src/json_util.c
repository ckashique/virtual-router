#include "json_util.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *vr_json_read_file(const char *path, size_t *size_out)
{
	FILE *fp;
	char *data;
	long size;

	if (path == NULL || size_out == NULL)
		return NULL;

	fp = fopen(path, "rb");
	if (fp == NULL)
		return NULL;

	if (fseek(fp, 0, SEEK_END) != 0) {
		fclose(fp);
		return NULL;
	}

	size = ftell(fp);
	if (size < 0) {
		fclose(fp);
		return NULL;
	}

	if (fseek(fp, 0, SEEK_SET) != 0) {
		fclose(fp);
		return NULL;
	}

	data = malloc((size_t)size + 1);
	if (data == NULL) {
		fclose(fp);
		return NULL;
	}

	if (fread(data, 1, (size_t)size, fp) != (size_t)size) {
		free(data);
		fclose(fp);
		return NULL;
	}

	data[size] = '\0';
	fclose(fp);
	*size_out = (size_t)size;
	return data;
}

static const char *skip_ws(const char *p, const char *end)
{
	while (p < end && isspace((unsigned char)*p))
		p++;

	return p;
}

static int parse_string_value(const char *p, const char *end, char *out,
			      size_t out_len, const char **next)
{
	size_t i;

	p = skip_ws(p, end);
	if (p >= end || *p != '"')
		return -1;
	p++;

	for (i = 0; p < end && *p != '"'; p++) {
		if (i + 1 >= out_len)
			return -1;
		out[i++] = *p;
	}

	if (p >= end || *p != '"')
		return -1;

	out[i] = '\0';
	*next = p + 1;
	return 0;
}

static int parse_uint_value(const char *p, const char *end, unsigned *out,
			    unsigned max_value, const char **next)
{
	char *endptr;
	unsigned long value;

	p = skip_ws(p, end);
	if (p >= end || !isdigit((unsigned char)*p))
		return -1;

	errno = 0;
	value = strtoul(p, &endptr, 10);
	if (errno != 0 || endptr == p)
		return -1;
	if (max_value < UINT_MAX && value > max_value)
		return -1;

	*out = (unsigned)value;
	*next = endptr;
	return 0;
}

const char *vr_json_find_key_array(const char *json, const char *end,
				   const char *key)
{
	char pattern[64];
	const char *key_pos;
	const char *p;
	size_t pattern_len;

	snprintf(pattern, sizeof(pattern), "\"%s\"", key);
	pattern_len = strlen(pattern);

	key_pos = json;
	while (key_pos < end) {
		size_t remaining = (size_t)(end - key_pos);

		if (remaining < pattern_len)
			return NULL;

		key_pos = memmem(key_pos, remaining, pattern, pattern_len);
		if (key_pos == NULL)
			return NULL;

		p = skip_ws(key_pos + pattern_len, end);
		if (p < end && *p == ':') {
			p = skip_ws(p + 1, end);
			if (p < end && *p == '[')
				return p + 1;
		}

		key_pos += pattern_len;
	}

	return NULL;
}

const char *vr_json_next_object(const char *p, const char *end,
				const char **obj_end)
{
	int depth;

	p = skip_ws(p, end);
	if (p >= end || *p != '{')
		return NULL;

	depth = 0;
	for (; p < end; p++) {
		if (*p == '{')
			depth++;
		else if (*p == '}') {
			depth--;
			if (depth == 0) {
				*obj_end = p + 1;
				return skip_ws(p + 1, end);
			}
		}
	}

	return NULL;
}

static const char *object_field_value(const char *obj_start,
					const char *obj_end, const char *key)
{
	char pattern[64];
	const char *key_pos;
	const char *p;
	size_t pattern_len;

	snprintf(pattern, sizeof(pattern), "\"%s\"", key);
	pattern_len = strlen(pattern);

	key_pos = obj_start;
	while (key_pos < obj_end) {
		size_t remaining = (size_t)(obj_end - key_pos);

		if (remaining < pattern_len)
			return NULL;

		key_pos = memmem(key_pos, remaining, pattern, pattern_len);
		if (key_pos == NULL || key_pos >= obj_end)
			return NULL;

		p = skip_ws(key_pos + pattern_len, obj_end);
		if (p < obj_end && *p == ':')
			return skip_ws(p + 1, obj_end);

		key_pos += pattern_len;
	}

	return NULL;
}

int vr_json_object_string(const char *obj_start, const char *obj_end,
			  const char *key, char *out, size_t out_len)
{
	const char *value = object_field_value(obj_start, obj_end, key);
	const char *next;

	if (value == NULL)
		return -1;

	return parse_string_value(value, obj_end, out, out_len, &next);
}

int vr_json_object_uint(const char *obj_start, const char *obj_end,
			const char *key, unsigned *out, unsigned max_value)
{
	const char *value = object_field_value(obj_start, obj_end, key);
	const char *next;

	if (value == NULL)
		return -1;

	return parse_uint_value(value, obj_end, out, max_value, &next);
}

const char *vr_json_array_next_object(const char **cursor, const char *end,
				      const char **obj_end)
{
	const char *p = *cursor;
	const char *obj_start;

	if (cursor == NULL || end == NULL || obj_end == NULL)
		return NULL;

	p = skip_ws(p, end);
	if (p >= end || *p == ']') {
		*cursor = p;
		return NULL;
	}
	if (*p == ',')
		p = skip_ws(p + 1, end);
	if (p >= end || *p == ']') {
		*cursor = p;
		return NULL;
	}

	obj_start = p;
	if (*obj_start != '{')
		return NULL;

	*obj_end = obj_start;
	p = vr_json_next_object(obj_start, end, obj_end);
	if (p == NULL)
		return NULL;

	*cursor = p;
	return obj_start;
}

int vr_json_at_array_end(const char *cursor, const char *end)
{
	cursor = skip_ws(cursor, end);
	return cursor >= end || *cursor == ']';
}
