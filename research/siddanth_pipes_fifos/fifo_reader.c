#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define FIFO_PATH "/tmp/myfifo"

int main(void) {
    char buffer[100];

    int fd = open(FIFO_PATH, O_RDONLY); // blocks until a writer opens it
    if (fd == -1) {
        perror("open");
        exit(1);
    }

    ssize_t n = read(fd, buffer, sizeof(buffer));
    if (n > 0) {
        printf("[Reader] Received: %s\n", buffer);
    }

    close(fd);
    unlink(FIFO_PATH); // remove the FIFO file
    return 0;
}
