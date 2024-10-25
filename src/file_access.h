#ifndef FILE_ACCESS_H
#define FILE_ACCESS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

int is_file_readable(const char *filename);
char *create_temp_file(const char *original_path);

#endif // FILE_ACCESS_H
