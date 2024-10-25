#include "safenoread.h"

int is_readable(const char *filename) {
    FILE *file = fopen(SAFENOREAD_PATH, "r");
    if (!file) return 1; // File doesn't exist, assume it's readable

    char line[PATH_MAX];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0; // Remove newline character
        if (strcmp(line, filename) == 0) {
            fclose(file);
            return 0; // Not readable
        }
    }
    fclose(file);
    return 1; // Readable
}
