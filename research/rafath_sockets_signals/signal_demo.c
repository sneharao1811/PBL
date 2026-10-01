#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

// Signal handler function
void handler(int sig) {
    if (sig == SIGUSR1) {
        printf("\n[Handler] Received SIGUSR1: hello from another process!\n");
    } else if (sig == SIGINT) {
        printf("\n[Handler] Received SIGINT (Ctrl+C): shutting down cleanly.\n");
        _exit(0);
    }
}

int main(void) {
    struct sigaction sa;
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGINT,  &sa, NULL);

    printf("Process running. PID = %d\n", getpid());
    printf("From another terminal run:  kill -SIGUSR1 %d\n", getpid());
    printf("Press Ctrl+C to exit.\n");

    while (1) {
        pause(); // sleep until a signal arrives
    }

    return 0;
}
