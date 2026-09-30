#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>      // O_CREAT, O_RDWR
#include <sys/mman.h>   // shm_open, mmap, munmap, shm_unlink
#include <sys/stat.h>   // mode constants
#include <sys/wait.h>   // wait
#include <unistd.h>     // fork, ftruncate, close

#define SHM_NAME "/ipc_demo_shm"
#define SHM_SIZE 4096

int main(void) {
    // 1. Create the shared memory object
    int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        perror("shm_open");
        exit(1);
    }

    // 2. Set its size
    if (ftruncate(fd, SHM_SIZE) == -1) {
        perror("ftruncate");
        exit(1);
    }

    // 3. Map it into this process's address space
    char *ptr = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    pid_t pid = fork();
    if (pid == 0) {
        // ---- CHILD: writer ----
        printf("[Child ] Writing to shared memory...\n");
        strcpy(ptr, "Hello from the child process!");
        munmap(ptr, SHM_SIZE);
        exit(0);
    } else {
        // ---- PARENT: reader ----
        wait(NULL); // wait until child has finished writing
        printf("[Parent] Read from shared memory: %s\n", ptr);

        // 5 and 6. Clean up
        munmap(ptr, SHM_SIZE);
        close(fd);
        shm_unlink(SHM_NAME);
    }

    return 0;
}
