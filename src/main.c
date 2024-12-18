#include "helpers.h"
#include <sys/wait.h>
#include <errno.h>

// trace process is used to keep track programs. the initial_pid is the id of the application.
void trace_process(pid_t initial_pid)
{
    int status; // this variable is used for getting the application status
    struct user_regs_struct regs; // this variable is used to extract the registers values
    pid_t current_pid; // current_pid is used when the app runs childs

    fprintf(stderr, "tracing %d\n", initial_pid);

    // set options for the initial process, to automatically trace the forks, vforks, and clones
    if (ptrace(PTRACE_SETOPTIONS, initial_pid, 0, PTRACE_O_TRACESYSGOOD | PTRACE_O_TRACEFORK | PTRACE_O_TRACEVFORK | PTRACE_O_TRACECLONE) == -1)
    {
        perror("ptrace SETOPTIONS failed for initial process");
        return;
    }

    // continue the initial process
    ptrace(PTRACE_SYSCALL, initial_pid, 0, 0);

    // tracing loop
    while (1)
    {
        // wait for any child process to stop
        current_pid = waitpid(-1, &status, __WALL);
        if (current_pid == -1)
        {
            if (errno == ECHILD)
            {
                // no more children to wait for
                break;
            }
            perror("waitpid failed");
            break;
        }

        // check for process termination
        if (WIFEXITED(status))
        {
            fprintf(stderr, "process %d terminated\n", current_pid);
            if (current_pid == initial_pid)
            {
                // if the initial process has exited, we're done
                break;
            }

            continue;
        }

        // check for stopped status, to handle syscall and events
        if (WIFSTOPPED(status))
        {
            int event = status >> 16;
            if (event == PTRACE_EVENT_FORK || event == PTRACE_EVENT_VFORK || event == PTRACE_EVENT_CLONE)
            {
                unsigned long new_pid;
                if (ptrace(PTRACE_GETEVENTMSG, current_pid, 0, &new_pid) != -1)
                {
                    fprintf(stderr, "new child process: %ld\n", new_pid);

                    // wait for the new process to stop
                    waitpid(new_pid, &status, __WALL);
                    if (ptrace(PTRACE_SETOPTIONS, new_pid, 0, PTRACE_O_TRACESYSGOOD | PTRACE_O_TRACEFORK | PTRACE_O_TRACEVFORK | PTRACE_O_TRACECLONE) == -1)
                    {
                        perror("ptrace SETOPTIONS failed for child");
                    }

                    // continue the new process
                    ptrace(PTRACE_SYSCALL, new_pid, 0, 0);
                }
            }
            else if ((status >> 8) == (SIGTRAP | 0x80)) // check for trap
            {
                if (ptrace(PTRACE_GETREGS, current_pid, 0, &regs) != -1)
                {
                    // handle the syscall
                    handle_syscall(current_pid, regs);
                }
            }
        }

        // continue the current process
        ptrace(PTRACE_SYSCALL, current_pid, 0, 0);
    }
}

int main(int argc, char *argv[])
{
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
