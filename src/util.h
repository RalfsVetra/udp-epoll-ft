#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>

int32_t get_file_size32(const char *path);
const char *get_file_name(const char *path);
int now_sec(void);

#endif /* UTIL_H */
