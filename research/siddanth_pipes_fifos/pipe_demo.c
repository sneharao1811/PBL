#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void) {
    int fd[2];
    char buffer[100];

    // 1. Create the pipe
    if (pipe(fd) == -1) {
        perror("pipe");
        exit(1);
    }

    // 2. Create the child process
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) {
        // ---------- CHILD: reader ----------
        close(fd[1]); // close unused write end

        ssize_t n = read(fd[0], buffer, sizeof(buffer) - 1);
        if (n > 0) {
            buffer[n] = '\0'; // make it a valid C string
            printf("[Child ] Received: %s\n", buffer);
        }

        close(fd[0]);
        exit(0);
    } else {
        // ---------- PARENT: writer ----------
        close(fd[0]); // close unused read end

        const char *msg = "Hello child, this is your parent!";
        printf("[Parent] Sending: %s\n", msg);
        write(fd[1], msg, strlen(msg));

        close(fd[1]); // signals end-of-data to reader
        wait(NULL);   // wait for child to finish
    }

    return 0;
}
