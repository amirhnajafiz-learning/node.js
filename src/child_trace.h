#ifndef CHILD_TRACE_H
#define CHILD_TRACE_H

#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void trace_child(pid_t child);
void handle_new_child(pid_t child);

#endif // CHILD_TRACE_H
