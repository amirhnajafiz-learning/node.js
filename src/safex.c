#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <errno.h>
#include <signal.h>
#include <dirent.h>
#include <limits.h>

#define SAFENOREAD_PATH "/home/<username>/.safenoread" // Change <username> appropriately

int is_file_readable(const char *filename) {
    FILE *file = fopen(SAFENOREAD_PATH, "r");
    if (!file) return 1; // File doesn't exist, assume it's readable

    char line[PATH_MAX];
    while (fgets(line, sizeof(line), file)) {
        // Remove newline character
        line[strcspn(line, "\n")] = 0;
        if (strcmp(line, filename) == 0) {
            fclose(file);
            return 0; // Not readable
        }
    }
    fclose(file);
    return 1; // Readable
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

void handle_syscall(pid_t child) {
    struct user_regs_struct regs;
    long syscall_num = ptrace(PTRACE_PEEKUSER, child, sizeof(long) * ORIG_RAX, NULL);

    if (syscall_num == SYS_open || syscall_num == SYS_openat) {
        char filename[PATH_MAX];
        // Get the filename argument
        ptrace(PTRACE_GETREGS, child, NULL, &regs);
        ptrace(PTRACE_PEEKDATA, child, regs.rdi, &filename); // Assuming filename is in rdi for x86_64

        if (is_file_readable(filename)) {
            // Allowed to read
            return;
        } else {
            // Deny read access
            ptrace(PTRACE_KILL, child, NULL, NULL);
            return;
        }
    } else if (syscall_num == SYS_creat || syscall_num == SYS_open || syscall_num == SYS_openat) {
        char filename[PATH_MAX];
        // Get the filename argument
        ptrace(PTRACE_GETREGS, child, NULL, &regs);
        ptrace(PTRACE_PEEKDATA, child, regs.rdi, &filename); // Assuming filename is in rdi for x86_64

        char *temp_file = create_temp_file(filename);
        if (temp_file) {
            // Redirect access to the temporary file
            ptrace(PTRACE_POKEDATA, child, regs.rdi, temp_file);
            free(temp_file);
        }
    }
}

void trace_child(pid_t child) {
    int status;
    while (1) {
        waitpid(child, &status, 0);
        if (WIFEXITED(status)) {
            break;
        }
        if (WIFSTOPPED(status)) {
            handle_syscall(child);
            ptrace(PTRACE_SYSCALL, child, NULL, NULL); // Resume the child
        }
    }
}

void handle_new_child(pid_t child) {
    // Trace the newly created child
    ptrace(PTRACE_ATTACH, child, NULL, NULL);
    trace_child(child);
    ptrace(PTRACE_DETACH, child, NULL, 0); // Detach after tracing
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <program> [args...]\n", argv[0]);
        return EXIT_FAILURE;
    }

    pid_t child = fork();
    if (child == 0) {
        // Child process
        ptrace(PTRACE_TRACEME);
        execvp(argv[1], &argv[1]);
        perror("execvp");
        return EXIT_FAILURE;
    } else {
        // Parent process
        // Enable tracing for fork, vfork, and clone
        ptrace(PTRACE_SETOPTIONS, child, 0, PTRACE_O_TRACEFORK | PTRACE_O_TRACEVFORK | PTRACE_O_TRACECLONE);
        
        int status;
        while (1) {
            wait(&status);
            if (WIFEXITED(status)) break;

            if (WIFSTOPPED(status)) {
                if (WSTOPSIG(status) == SIGTRAP) {
                    // Handle fork, vfork, clone events
                    int event = (status >> 16) & 0xffff;
                    if (event == PTRACE_EVENT_FORK || event == PTRACE_EVENT_VFORK || event == PTRACE_EVENT_CLONE) {
                        pid_t new_child = waitpid(-1, &status, WNOHANG);
                        if (new_child > 0) {
                            handle_new_child(new_child);
                        }
                    }
                } else {
                    handle_syscall(child);
                }
            }
            ptrace(PTRACE_SYSCALL, child, NULL, NULL); // Resume the original child
        }
    }
    return EXIT_SUCCESS;
}
