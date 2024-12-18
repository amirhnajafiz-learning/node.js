#define _GNU_SOURCE
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <sys/reg.h>
#include <sys/user.h>
#include <errno.h>
#include <signal.h>
#include <dirent.h>
#include <limits.h>

// maximum buffer size and the number of files per application
#define BUFFER_SIZE 256
#define MAX_FILES 100000

// define the safenoread file path
#define SAFENOREAD_PATH "/.safenoread"

int find_redirect(const char *filename);
char *remove_prefix(char *s, const char *prefix);
char *get_safenoread_path();
char *create_temp_file();
bool is_safenoread(const char *filename);
char *get_filename_from_addr(pid_t pid, long filename_addr);
bool filter(long syscall);
void handle_syscall(pid_t child, struct user_regs_struct regs);
