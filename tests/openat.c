#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SIZE 1024

int main() {
    int dirfd, fd;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    // Open the current directory
    dirfd = open(".", O_RDONLY);
    if (dirfd == -1) {
        perror("Error opening current directory");
        exit(1);
    }

    // Open the file using openat
    fd = openat(dirfd, "tests/files/auth", O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        close(dirfd);
        exit(1);
    }

    // Read from the file
    bytes_read = read(fd, buffer, BUFFER_SIZE - 1);
    if (bytes_read == -1) {
        perror("Error reading file");
        close(fd);
        close(dirfd);
        exit(1);
    }

    // Null-terminate the buffer and print its contents
    buffer[bytes_read] = '\0';
    printf("File contents:\n%s\n", buffer);

    // Close file descriptors
    close(fd);
    close(dirfd);

    return 0;
}
