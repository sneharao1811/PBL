#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#define SEM_NAME "/ipc_demo_sem"

static void print_block(const char *who) {
    for (int i = 1; i <= 3; i++) {
        printf("[%s] line %d\n", who, i);
        fflush(stdout);
        usleep(200000); // 0.2 second, to make interleaving visible
    }
}

int main(void) {
    // Create a binary semaphore with initial value 1
    sem_t *sem = sem_open(SEM_NAME, O_CREAT, 0666, 1);
    if (sem == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }

    pid_t pid = fork();
    if (pid == 0) {
        // ---- CHILD ----
        sem_wait(sem); // enter critical section
        print_block("Child ");
        sem_post(sem); // leave critical section

        sem_close(sem);
        exit(0);
    } else {
        // ---- PARENT ----
        sem_wait(sem); // enter critical section
        print_block("Parent");
        sem_post(sem); // leave critical section

        wait(NULL);
        sem_close(sem);
        sem_unlink(SEM_NAME); // remove the semaphore
    }

    return 0;
}
