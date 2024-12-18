#include "tracer.h"
#include <sys/wait.h>
#include <errno.h>

int main(int argc, char *argv[]) {
    // we need at least 1 input command
    if (argc < 2)
    {
        fprintf(stderr, "not enough arguments to run: %s <command>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // fork a new child to run the application
    pid_t child = fork();
    if (child == 0)
    {
        // the child runs the application
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        execvp(argv[1], &argv[1]);
        perror("execvp failed");
        exit(EXIT_FAILURE);
    }
    else if (child > 0)
    {
        // the parent, starts tracing using ptrace
        int status;
        waitpid(child, &status, 0);

        // set ptrace options
        if (ptrace(PTRACE_SETOPTIONS, child, 0, PTRACE_O_TRACESYSGOOD | PTRACE_O_TRACEFORK | PTRACE_O_TRACEVFORK | PTRACE_O_TRACECLONE) == -1)
        {
            perror("ptrace SETOPTIONS failed");
            exit(EXIT_FAILURE);
        }

        // start tracing the child process (application)
        trace_process(child);
    }
    else
    {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}
