# safex

Safex is a program designed to create a secure environment, known as a sandbox, for applications. It redirects any file writing operations made by the applications to the `/tmp` directory. Additionally, Safex monitors file access and blocks any attempts by the application to access files listed in the `~/.safenoread` file.

## Compilation and Usage

To compile and use the Safex program, open a terminal and run the `make` command. This will generate the `safex` executable. You can then use this executable to sandbox an application. For example, to run the text editor Gedit with the sandboxing feature, you would enter the following command:

```
./safex gedit myfile
```

After running the application, you can check the `/tmp` directory for files that have been redirected, which will be named in the format `safex.XXRTFSE`.

## Implementation Details

Safex utilizes the `ptrace` system call to monitor the processes it is controlling. In the `main.c` file (specifically lines 91–110), a new process is created to run the specified application, while the parent process is responsible for tracing this child process. On line 104, options are set to monitor `fork`, `vfork`, and `clone` operations.

```c
if (ptrace(PTRACE_SETOPTIONS, child, 0, PTRACE_O_TRACESYSGOOD | PTRACE_O_TRACEFORK | PTRACE_O_TRACEVFORK | PTRACE_O_TRACECLONE) == -1) {
    perror("ptrace SETOPTIONS failed");
    exit(EXIT_FAILURE);
}
```

The program then calls the `trace_process` function, passing the child process's PID to monitor its system calls. Within the `trace_process` function, system calls are tracked using:

```c
ptrace(PTRACE_SYSCALL, initial_pid, 0, 0);
```

A loop is used to extract the system calls and invoke a function called `handle_syscall` to monitor the `open`, `openat`, and `write` system calls.

To trace child processes, the program examines events triggered during system calls. Since the event type is contained in the top 16 bits, it shifts the bits to identify the event and checks for any `fork`, `vfork`, or `clone` events. If such an event occurs, the program calls `ptrace` on the newly created child process.

```c
int event = status >> 16;
if (event == PTRACE_EVENT_FORK || event == PTRACE_EVENT_VFORK || event == PTRACE_EVENT_CLONE) {
    unsigned long new_pid;
    if (ptrace(PTRACE_GETEVENTMSG, current_pid, 0, &new_pid) != -1) {
        fprintf(stderr, "new child process: %ld\n", new_pid);
        waitpid(new_pid, &status, __WALL);
        if (ptrace(PTRACE_SETOPTIONS, new_pid, 0, PTRACE_O_TRACESYSGOOD | PTRACE_O_TRACEFORK | PTRACE_O_TRACEVFORK | PTRACE_O_TRACECLONE) == -1) {
            perror("ptrace SETOPTIONS failed for child");
        }
        ptrace(PTRACE_SYSCALL, new_pid, 0, 0);
    }
}
```

Next, the program checks the status of each traced process:

```c
// Wait for any child process to stop
current_pid = waitpid(-1, &status, __WALL);
if (current_pid == -1) {
    if (errno == ECHILD) {
        // No more children to wait for
        break;
    }
    perror("waitpid failed");
    break;
}

// Check for process termination
if (WIFEXITED(status)) {
    fprintf(stderr, "Process %d terminated\n", current_pid);
    if (current_pid == initial_pid) {
        // If the initial process has exited, we're done
        break;
    }
    continue;
}

// Check for stopped status to handle system calls and events
if (WIFSTOPPED(status)) {
    int event = status >> 16;
    if (event == PTRACE_EVENT_FORK || event == PTRACE_EVENT_VFORK || event == PTRACE_EVENT_CLONE) {
        // Handle child events
    } else if ((status >> 8) == (SIGTRAP | 0x80)) { // Check for trap
        if (ptrace(PTRACE_GETREGS, current_pid, 0, &regs) != -1) {
            // Handle the system call
            handle_syscall(current_pid, regs);
        }
    }
}

// Continue the current process
ptrace(PTRACE_SYSCALL, current_pid, 0, 0);
```

Further operations are carried out within helper functions.

### Helper Functions

The `helper.c` file includes functions that filter the `open` and `openat` system calls. It checks against the `.safenoread` file to block access to restricted files. If a write request is made to a file that is permitted, the program generates a temporary file in the `/tmp` directory using `mkstemp` and adjusts the process's register values to redirect the write operation.

First, the program retrieves the register data:

```c
// Determine the system call type
long syscall = regs.orig_rax;
```

The filtering function is then called to capture only the necessary system calls. The program extracts the filename and its associated flags from the registers:

```c
// Retrieve the filename and flags address
long filename_addr = regs.rsi;
long flags = regs.rdx;

// Get the filename from its address
char *filename = get_filename_from_addr(child, filename_addr);
if (filename == NULL) {
    return;
}
```

Next, the program checks if the filename exists in the `.safenoread` file. If it does not, the program examines the flags for `(flags & O_WRONLY) || (flags & O_RDWR)`, indicating a write operation. If a write operation is detected, the program creates a temporary file and replaces the original filename.

```c
// Set the target file for the process to access
for (size_t i = 0; i < strlen(target_file); i += sizeof(long)) {
    long data = 0;
    memcpy(&data, target_file + i, sizeof(long));
    ptrace(PTRACE_POKEDATA, child, regs.rsi + i, data);
}
```

A custom structure called `FileMap` is defined to store the original filenames along with their corresponding temporary redirects. This structure has a default limit of 10,000 entries, which can be increased in `helpers.h`.

```c
// Structure to track original files and their redirects
struct FileMap {
    char original_filename[PATH_MAX];
    char tmp_filename[PATH_MAX];
};

struct FileMap redirects[MAX_FILES];
int redirect_count = 0;
```

This mapping is used to ensure that multiple accesses to the same file are redirected to the correct temporary file.

### Safenoread File

The `~/.safenoread` file is located using the `HOME` environment variable. If the `.safenoread` file is not in your home directory, you should update its path in `helpers.h`. For example, if it is located in `home/username/Desktop/.safenoread`, set `SAFENOREAD_PATH` to `Desktop/.safenoread`.

Make sure that the file paths specified in `.safenoread` are absolute paths to correctly filter the intended files.

## Testing

To test the functionality of Safex, run the command `make test`. This will use sample applications located in the `bin` directory to verify that Safex operates as expected.
