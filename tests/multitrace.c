#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <sys/syscall.h>
#include <string.h>
#include <errno.h>

#define NUM_CHILDREN 5

void compute(int child_num) {
    printf("Child %d (PID %d): Starting computation\n", child_num, getpid());
    for (int i = 0; i < 3; i++) {
        printf("Child %d: Iteration %d\n", child_num, i);
        sleep(1);
    }
    printf("Child %d: Finished computation\n", child_num);
}

void trace_syscalls(pid_t child_pid) {
    int status;
    struct user_regs_struct regs;

    printf("Tracer (PID %d): Attaching to child %d\n", getpid(), child_pid);

    if (ptrace(PTRACE_ATTACH, child_pid, NULL, NULL) == -1) {
        perror("ptrace(ATTACH)");
        exit(1);
    }

    waitpid(child_pid, &status, 0);

    ptrace(PTRACE_SETOPTIONS, child_pid, 0, PTRACE_O_TRACESYSGOOD);

    while (1) {
        if (ptrace(PTRACE_SYSCALL, child_pid, 0, 0) == -1) {
            perror("ptrace(SYSCALL)");
            break;
        }

        if (waitpid(child_pid, &status, 0) == -1) {
            perror("waitpid");
            break;
        }

        if (WIFEXITED(status)) {
            printf("Tracer: Child %d has exited with status %d\n", child_pid, WEXITSTATUS(status));
            break;
        }

        if (WIFSTOPPED(status) && WSTOPSIG(status) & 0x80) {
            if (ptrace(PTRACE_GETREGS, child_pid, 0, &regs) == -1) {
                perror("ptrace(GETREGS)");
                break;
            }
            printf("Tracer: Child %d made syscall %lld\n", child_pid, regs.orig_rax);
        }
    }

    ptrace(PTRACE_DETACH, child_pid, NULL, NULL);
    printf("Tracer: Finished tracing child %d\n", child_pid);
}

int main() {
    pid_t pids[NUM_CHILDREN];
    pid_t tracer_pids[NUM_CHILDREN];

    for (int i = 0; i < NUM_CHILDREN; i++) {
        pids[i] = fork();
        if (pids[i] == 0) {
            // Child process
            compute(i + 1);
            exit(0);
        } else if (pids[i] > 0) {
            // Parent process
            printf("Parent: Created child %d with PID %d\n", i + 1, pids[i]);
            tracer_pids[i] = fork();
            if (tracer_pids[i] == 0) {
                // Tracer process
                trace_syscalls(pids[i]);
                exit(0);
            }
        } else {
            perror("fork");
            exit(1);
        }
    }

    // Wait for all children and tracers to finish
    for (int i = 0; i < NUM_CHILDREN * 2; i++) {
        wait(NULL);
    }

    printf("Parent: All children and tracers have finished\n");
    return 0;
}