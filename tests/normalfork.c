#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>

int main()
{
    pid_t pid = fork();
    if (pid == 0)
    {
        const char *filename = "tests/files/db";

        // Test opening the file for reading
        printf("Trying to open %s for reading...\n", filename);
        FILE *file = fopen(filename, "r");
        if (file)
        {
            printf("Successfully opened %s for reading.\n", filename);
            fclose(file);
        }
        else
        {
            printf("Error opening file for reading");
        }

        // Test opening the file for writing
        printf("Trying to open %s for writing...\n", filename);
        int fd = open(filename, O_WRONLY | O_CREAT);
        if (fd != -1)
        {
            printf("Successfully opened %s for writing.\n", filename);

            const char *content = "edited";
            write(fd, content, strlen(content));

            close(fd);
        }
        else
        {
            printf("Error opening file for writing");
        }

        return EXIT_SUCCESS;
    }
    else
    {
        if (pid == -1)
        {
            printf("Error fork");

            return 1;
        }

        int status = 0;

        wait(&status);

        return status;
    }
}
