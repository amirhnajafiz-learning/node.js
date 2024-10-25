#include "file_access.h"

int is_file_readable(const char *filename) {
    return is_readable(filename);
}

char *create_temp_file(const char *original_path) {
    char template[] = "/tmp/safex_XXXXXX"; // Temporary file template
    int fd = mkstemp(template);
    if (fd == -1) {
        perror("mkstemp");
        return NULL;
    }
    close(fd);
    return strdup(template);
}
