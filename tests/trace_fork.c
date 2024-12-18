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

void trace_syscalls(pid_t pid);

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <command> [args...]\n", argv[0]);
        exit(1);
    }

    pid_t child = fork();

    if (child == 0)
    {
        // Child process
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        execvp(argv[1], &argv[1]);
        perror("execvp");
        exit(1);
    }
    else if (child > 0)
    {
        // Parent process
        printf("Tracing process %d\n", child);
        trace_syscalls(child);
    }
    else
    {
        perror("fork");
        exit(1);
    }

    return 0;
}

void trace_syscalls(pid_t pid)
{
    int status;
    struct user_regs_struct regs;

    waitpid(pid, &status, 0);
    ptrace(PTRACE_SETOPTIONS, pid, 0, PTRACE_O_TRACEFORK | PTRACE_O_TRACEVFORK | PTRACE_O_TRACECLONE);

    while (1)
    {
        if (ptrace(PTRACE_SYSCALL, pid, 0, 0) == -1)
        {
            perror("ptrace");
            break;
        }
        if (waitpid(pid, &status, 0) == -1)
        {
            perror("waitpid");
            break;
        }

        ptrace(PTRACE_GETREGS, pid, 0, &regs);
        printf("[%d] Syscall: %lld\n", pid, regs.orig_rax);

        if (WIFEXITED(status))
        {
            printf("Process %d exited\n", pid);
            break;
        }

        if (status >> 8 == (SIGTRAP | (PTRACE_EVENT_FORK << 8)) ||
            status >> 8 == (SIGTRAP | (PTRACE_EVENT_VFORK << 8)) ||
            status >> 8 == (SIGTRAP | (PTRACE_EVENT_CLONE << 8)))
        {
            unsigned long new_pid;
            ptrace(PTRACE_GETEVENTMSG, pid, 0, &new_pid);
            printf("New child process: %ld\n", new_pid);
            trace_syscalls(new_pid);
        }
    }
}
