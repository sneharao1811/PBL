#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>

struct msg_buffer {
    long mtype;
    char mtext[100];
};

int main(void) {
    // Generate a unique key (both programs must use the same path and id)
    key_t key = ftok("/tmp", 65);
    if (key == -1) {
        perror("ftok");
        exit(1);
    }

    // Create the queue (or open it if it already exists)
    int msgid = msgget(key, 0666 | IPC_CREAT);
    if (msgid == -1) {
        perror("msgget");
        exit(1);
    }

    struct msg_buffer message;
    message.mtype = 1; // message type 1

    printf("Enter a message to send: ");
    fgets(message.mtext, sizeof(message.mtext), stdin);
    message.mtext[strcspn(message.mtext, "\n")] = '\0'; // remove newline

    if (msgsnd(msgid, &message, sizeof(message.mtext), 0) == -1) {
        perror("msgsnd");
        exit(1);
    }

    printf("Message sent: %s\n", message.mtext);
    return 0;
}
