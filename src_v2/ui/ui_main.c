#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <mqueue.h>
#include "../include/protocol.h"

static volatile sig_atomic_t keep_running = 1;
static mqd_t mq_ui2core = (mqd_t)-1;
static mqd_t mq_core2ui = (mqd_t)-1;

static void handle_signal(int sig) {
    (void)sig;
    keep_running = 0;
}

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    // Open existing message queues created by supervisor
    mq_ui2core = mq_open(QUEUE_UI_TO_CORE, O_WRONLY);
    if (mq_ui2core == (mqd_t)-1) {
        perror("[UI] Failed to open QUEUE_UI_TO_CORE");
        return 1;
    }

    mq_core2ui = mq_open(QUEUE_CORE_TO_UI, O_RDONLY);
    if (mq_core2ui == (mqd_t)-1) {
        perror("[UI] Failed to open QUEUE_CORE_TO_UI");
        mq_close(mq_ui2core);
        return 1;
    }

    int is_interactive = isatty(fileno(stdin));

    if (is_interactive) {
        printf("====================================================\n");
        printf(" Welcome to the Multi-Process Simulator (POSIX IPC) \n");
        printf(" Architecture: UI [PID %d] <-> Core <-> Logger    \n", getpid());
        printf(" Available commands: PUSH <val>, POP, ADD, STATUS, EXIT\n");
        printf("====================================================\n");
    }

    char input[128];
    char cmd[32];
    int arg;
    uint32_t req_counter = 0;

    while (keep_running) {
        if (is_interactive) {
            printf("\nSIM> ");
            fflush(stdout);
        }

        if (!fgets(input, sizeof(input), stdin)) {
            // EOF detected
            break;
        }

        input[strcspn(input, "\r\n")] = '\0';
        if (strlen(input) == 0) continue;

        cmd[0] = '\0';
        arg = 0;
        int parsed = sscanf(input, "%31s %d", cmd, &arg);
        if (parsed < 1) continue;

        ipc_req_t req;
        memset(&req, 0, sizeof(req));
        req.req_id = ++req_counter;
        req.sender_pid = getpid();

        if (strcmp(cmd, "EXIT") == 0) {
            req.opcode = CMD_EXIT;
        } else if (strcmp(cmd, "PUSH") == 0) {
            req.opcode = CMD_PUSH;
            req.operand = arg;
        } else if (strcmp(cmd, "POP") == 0) {
            req.opcode = CMD_POP;
        } else if (strcmp(cmd, "ADD") == 0) {
            req.opcode = CMD_ADD;
        } else if (strcmp(cmd, "STATUS") == 0) {
            req.opcode = CMD_STATUS;
        } else {
            if (is_interactive) {
                printf("Unknown command: %s\n", cmd);
            }
            continue;
        }

        // Send to Core
        if (mq_send(mq_ui2core, (const char *)&req, sizeof(ipc_req_t), 0) == -1) {
            perror("[UI] mq_send to Core failed");
            break;
        }

        // Await Core response
        ipc_resp_t resp;
        ssize_t bytes = mq_receive(mq_core2ui, (char *)&resp, sizeof(ipc_resp_t), NULL);
        if (bytes >= 0) {
            if (strlen(resp.display_msg) > 0) {
                printf("%s\n", resp.display_msg);
            }
            if (resp.status == STATUS_SHUTDOWN || req.opcode == CMD_EXIT) {
                if (is_interactive) printf("Goodbye!\n");
                break;
            }
        } else {
            if (!keep_running) break;
            perror("[UI] mq_receive from Core failed");
        }
    }

    mq_close(mq_ui2core);
    mq_close(mq_core2ui);

    return 0;
}
