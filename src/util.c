#include <sys/stat.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

int32_t get_file_size32(const char *path) {
    struct stat st;
    if (stat(path, &st) == -1)
        return -1;

    if (st.st_size > UINT32_MAX)
        return -1;

    return (int32_t)st.st_size;
}

const char *get_file_name(const char *path) {
	if (!path)
		return "";
	
    const char *slash = strrchr(path, '/');
    return slash ? slash + 1 : path;
}

int now_sec(void) {
    time_t seconds;
    time(&seconds);
    return (int)seconds;
}
