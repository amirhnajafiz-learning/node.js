#include "helpers.h"

struct FileMap redirects[MAX_FILES];
int redirect_count = 0;

// find redirect is used to get a redirect pair data if exists.
// it returns an index in redirects array. returns -1 if not exists.
int find_redirect(const char *filename)
{
    for (int i = 0; i < redirect_count; i++)
    {
        if (strcmp(redirects[i].original_filename, filename) == 0 || strcmp(redirects[i].tmp_filename, filename) == 0)
        {
            return i;
        }
    }

    return -1;
}

// remove prefix is a helper function that is used to trim filenames.
char *remove_prefix(char *s, const char *prefix)
{
    size_t prefix_len = strlen(prefix);
    if (strncmp(s, prefix, prefix_len) == 0)
    {
        memmove(s, s + prefix_len, strlen(s) - prefix_len + 1);
    }

    return s;
}

// get safenoread path tries to get the absolute path for ~/.safenoread
char *get_safenoread_path()
{
    const char *home = getenv("HOME"); // get the user's home directory path
    if (home == NULL)
    {
        return NULL; // return NULL if HOME environment variable is not found
    }

    // allocate memory for the full path
    char *path = malloc(strlen(home) + strlen(SAFENOREAD_PATH) + 1);
    if (path == NULL)
    {
        return NULL; // return NULL if memory allocation fails
    }

    // construct the path
    strcpy(path, home);
    strcat(path, SAFENOREAD_PATH);

    return path;
}

// this function creates a temporary file by using mkstemp.
char *create_temp_file()
{
    char *temp_filename = malloc(PATH_MAX);
    if (!temp_filename)
    {
        perror("malloc");
        return NULL;
    }

    // using mkstemp to create a temp file
    snprintf(temp_filename, PATH_MAX, "/tmp/safex.XXXXX");
    int fd = mkstemp(temp_filename);
    if (fd == -1)
    {
        perror("mkstemp failed");
        free(temp_filename);

        return NULL;
    }

    close(fd);

    return temp_filename; // return the newly created temp file
}

// is safenoread returns true if the file is in .safenoread file.
bool is_safenoread(const char *filename)
{
    char *safenoread_path = get_safenoread_path();
    if (!safenoread_path)
    {
        return false; // no restrictions if safenoread path can't be retrieved
    }

    FILE *file = fopen(safenoread_path, "r");
    free(safenoread_path);
    if (!file)
        return false; // no restrictions if file doesn't exist

    char line[PATH_MAX];
    while (fgets(line, sizeof(line), file))
    {
        line[strcspn(line, "\n")] = 0; // remove newline
        if (strcmp(line, filename) == 0)
        {
            fclose(file);
            return true; // file is not readable
        }
    }

    fclose(file);

    return false; // file is readable
}

// get filename from addrress gets a child pid and a filename location
// then it will read that filename and stores it inside memory.
char *get_filename_from_addr(pid_t pid, long filename_addr)
{
    char *filename = malloc(BUFFER_SIZE);
    if (!filename)
    {
        perror("failed to malloc");
        return NULL;
    }

    size_t total_read = 0;
    long data;
    size_t offset = 0;

    while (total_read < BUFFER_SIZE - 1)
    { // leave space for null terminator
        data = ptrace(PTRACE_PEEKDATA, pid, filename_addr + offset, NULL);
        if (data == -1 && errno)
        {
            perror("ptrace PEEKDATA failed");
            free(filename);
            return NULL;
        }

        // copy the data into the filename buffer
        memcpy(filename + total_read, &data, sizeof(data));
        total_read += sizeof(data);

        // check for null terminator in the last chunk
        if (memchr(&data, '\0', sizeof(data)))
        {
            break;
        }

        offset += sizeof(data);
    }

    filename[total_read] = '\0'; // null-terminate the string

    return remove_prefix(filename, "./");
}

// filter to check if syscall is in the array of our needed system calls
bool filter(long syscall)
{
    long syscalls[] = {SYS_open, SYS_openat};

    for (size_t i = 0; i < sizeof(syscalls) / sizeof(syscalls[0]); i++)
    {
        if (syscall == syscalls[i])
        {
            return true;
        }
    }

    return false;
}

// the handle syscall function processes system calls for the traced process.
// it gets pid of the application as input.
void handle_syscall(pid_t child, struct user_regs_struct regs)
{
    // determine the syscall type
    long syscall = regs.orig_rax;

    // filter the system call
    if (filter(syscall))
    {
        // retrieve filename and flags address
        long filename_addr = regs.rsi;
        long flags = regs.rdx;

        // get the filename from its address
        char *filename = get_filename_from_addr(child, filename_addr);
        if (filename == NULL)
        {
            return;
        }

        // check to see if the file is in safenoread
        if (is_safenoread(filename)) // if this return true, it means the file cannot be accessed
        {
            fprintf(stderr, "malicious behavior by %d to access %s\n", child, filename);

            regs.orig_rax = -1; // set return -1 for child
            regs.rdi = ENOSYS;  // set error number to ENOSYS (bad system call)

            // write modified registers back to the child process
            ptrace(PTRACE_SETREGS, child, NULL, &regs);

            return;
        }
        else
        {
            // target file is the file that eventually will be returned to the process
            char *target_file = filename;

            // first we check to see if we already redirected this file access or not
            int redirect_index = find_redirect(filename);
            if (redirect_index != -1)
            {
                target_file = redirects[redirect_index].tmp_filename;
            }
            else if ((flags & O_WRONLY) || (flags & O_RDWR))
            {
                // create a new file to redirect the access on write operations
                char *temp_file = create_temp_file(filename);

                strncpy(redirects[redirect_count].original_filename, filename, PATH_MAX);
                strncpy(redirects[redirect_count].tmp_filename, temp_file, PATH_MAX);
                redirect_count++;

                fprintf(stderr, "redirected %d access from %s to %s\n", child, filename, temp_file);

                target_file = temp_file;
            }

            // set the target file for the process to access
            for (size_t i = 0; i < strlen(target_file); i += sizeof(long))
            {
                long data = 0;
                memcpy(&data, target_file + i, sizeof(long));
                ptrace(PTRACE_POKEDATA, child, regs.rsi + i, data);
            }
        }
    }
}
