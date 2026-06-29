#ifndef VR_JSON_UTIL_H
#define VR_JSON_UTIL_H

#include <stddef.h>

char *vr_json_read_file(const char *path, size_t *size_out);

const char *vr_json_find_key_array(const char *json, const char *end,
				   const char *key);

const char *vr_json_next_object(const char *p, const char *end,
				const char **obj_end);

int vr_json_object_string(const char *obj_start, const char *obj_end,
			  const char *key, char *out, size_t out_len);

int vr_json_object_uint(const char *obj_start, const char *obj_end,
			const char *key, unsigned *out, unsigned max_value);

const char *vr_json_array_next_object(const char **cursor, const char *end,
				      const char **obj_end);

int vr_json_at_array_end(const char *cursor, const char *end);

#endif /* VR_JSON_UTIL_H */
