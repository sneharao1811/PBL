#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080

int main(void) {
    // 1. Create the socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("socket");
        exit(1);
    }

    // 2. Describe the server to connect to
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port   = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    // 3. Connect
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect");
        exit(1);
    }

    printf("[Client] Connected to server.\n");

    // 4. Send a message
    const char *msg = "Hello server, this is the client!";
    write(sock, msg, strlen(msg));
    printf("[Client] Sent: %s\n", msg);

    // 5. Receive the reply
    char buffer[1024] = {0};
    ssize_t n = read(sock, buffer, sizeof(buffer) - 1);
    if (n > 0) {
        printf("[Client] Received: %s\n", buffer);
    }

    close(sock);
    return 0;
}
