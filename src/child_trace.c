#include "child_trace.h"
#include "file_access.h"

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
