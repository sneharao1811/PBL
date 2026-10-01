#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080

int main(void) {
    // 1. Create the socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket");
        exit(1);
    }

    // Allow quick reuse of the port after restarting
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 2. Bind to IP + port
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY; // accept on any local address
    addr.sin_port        = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("bind");
        exit(1);
    }

    // 3. Listen (queue up to 3 pending connections)
    if (listen(server_fd, 3) == -1) {
        perror("listen");
        exit(1);
    }

    printf("[Server] Listening on port %d...\n", PORT);

    // 4. Accept a client connection (blocks until a client connects)
    int client_fd = accept(server_fd, NULL, NULL);
    if (client_fd == -1) {
        perror("accept");
        exit(1);
    }

    printf("[Server] Client connected.\n");

    // 5. Receive a message
    char buffer[1024] = {0};
    ssize_t n = read(client_fd, buffer, sizeof(buffer) - 1);
    if (n > 0) {
        printf("[Server] Received: %s\n", buffer);
    }

    // 6. Send a reply
    const char *reply = "Hello client, message received!";
    write(client_fd, reply, strlen(reply));
    printf("[Server] Reply sent.\n");

    // 7. Close
    close(client_fd);
    close(server_fd);
    return 0;
}
