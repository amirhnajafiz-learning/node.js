#include "child_trace.h"
#include "safenoread.h"

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
        
        trace_child(child);
    }
    return EXIT_SUCCESS;
}
