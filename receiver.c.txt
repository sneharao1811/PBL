#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>

struct msg_buffer {
    long mtype;
    char mtext[100];
};

int main(void) {
    key_t key = ftok("/tmp", 65); // same key as the sender
    if (key == -1) {
        perror("ftok");
        exit(1);
    }

    int msgid = msgget(key, 0666 | IPC_CREAT);
    if (msgid == -1) {
        perror("msgget");
        exit(1);
    }

    struct msg_buffer message;

    // Receive a message of type 1 (blocks until one arrives)
    if (msgrcv(msgid, &message, sizeof(message.mtext), 1, 0) == -1) {
        perror("msgrcv");
        exit(1);
    }

    printf("Message received: %s\n", message.mtext);

    // Delete the queue
    msgctl(msgid, IPC_RMID, NULL);
    return 0;
}
