#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#define FIFO_PATH "/tmp/myfifo"

int main(void) {
    // Create the FIFO (ignore the error if it already exists)
    mkfifo(FIFO_PATH, 0666);
    printf("[Writer] Waiting for a reader to connect...\n");

    int fd = open(FIFO_PATH, O_WRONLY); // blocks until a reader opens it
    if (fd == -1) {
        perror("open");
        exit(1);
    }

    const char *msg = "Hello through the FIFO!";
    write(fd, msg, strlen(msg) + 1); // +1 to include the '\0'
    printf("[Writer] Sent: %s\n", msg);

    close(fd);
    return 0;
}
