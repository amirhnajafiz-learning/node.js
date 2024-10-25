#ifndef SAFENOREAD_H
#define SAFENOREAD_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define SAFENOREAD_PATH "/home/<username>/.safenoread" // Change <username> appropriately

int is_readable(const char *filename);

#endif // SAFENOREAD_H
