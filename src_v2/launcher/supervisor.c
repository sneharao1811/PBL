#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <mqueue.h>
#include "../include/protocol.h"

static pid_t pid_logger = -1;
static pid_t pid_core   = -1;
static pid_t pid_ui     = -1;

static void cleanup_queues(void) {
    mq_unlink(QUEUE_UI_TO_CORE);
    mq_unlink(QUEUE_CORE_TO_UI);
    mq_unlink(QUEUE_TO_LOGGER);
}

static void term_children(void) {
    if (pid_ui > 0)     kill(pid_ui, SIGTERM);
    if (pid_core > 0)   kill(pid_core, SIGTERM);
    if (pid_logger > 0) kill(pid_logger, SIGTERM);
}

static void handle_sigint(int sig) {
    (void)sig;
    term_children();
    cleanup_queues();
    exit(0);
}

int main(int argc, char *argv[]) {
    signal(SIGINT, handle_sigint);
    signal(SIGTERM, handle_sigint);

    // Clean any prior dangling queues
    cleanup_queues();

    // 1. Initialize POSIX Message Queues with attributes
    struct mq_attr attr_req;
    attr_req.mq_flags = 0;
    attr_req.mq_maxmsg = MAX_QUEUE_MSGS;
    attr_req.mq_msgsize = sizeof(ipc_req_t);
    attr_req.mq_curmsgs = 0;

    struct mq_attr attr_resp;
    attr_resp.mq_flags = 0;
    attr_resp.mq_maxmsg = MAX_QUEUE_MSGS;
    attr_resp.mq_msgsize = sizeof(ipc_resp_t);
    attr_resp.mq_curmsgs = 0;

    struct mq_attr attr_log;
    attr_log.mq_flags = 0;
    attr_log.mq_maxmsg = MAX_QUEUE_MSGS;
    attr_log.mq_msgsize = sizeof(ipc_log_t);
    attr_log.mq_curmsgs = 0;

    mqd_t q1 = mq_open(QUEUE_UI_TO_CORE, O_CREAT | O_RDWR, 0666, &attr_req);
    mqd_t q2 = mq_open(QUEUE_CORE_TO_UI, O_CREAT | O_RDWR, 0666, &attr_resp);
    mqd_t q3 = mq_open(QUEUE_TO_LOGGER,  O_CREAT | O_RDWR, 0666, &attr_log);

    if (q1 == (mqd_t)-1 || q2 == (mqd_t)-1 || q3 == (mqd_t)-1) {
        perror("[Supervisor] Error creating POSIX Message Queues");
        cleanup_queues();
        return 1;
    }

    // 2. Fork and spawn Process 3: Logger
    pid_logger = fork();
    if (pid_logger == 0) {
        execl("./bin/logger_proc", "logger_proc", NULL);
        perror("[Supervisor] Failed to exec logger_proc");
        exit(1);
    }

    usleep(50000);

    // 3. Fork and spawn Process 2: Core Engine
    pid_core = fork();
    if (pid_core == 0) {
        execl("./bin/core_proc", "core_proc", NULL);
        perror("[Supervisor] Failed to exec core_proc");
        exit(1);
    }

    usleep(50000);

    // 4. Fork and spawn Process 1: UI
    pid_ui = fork();
    if (pid_ui == 0) {
        if (argc > 1) {
            execl("./bin/ui_proc", "ui_proc", argv[1], NULL);
        } else {
            execl("./bin/ui_proc", "ui_proc", NULL);
        }
        perror("[Supervisor] Failed to exec ui_proc");
        exit(1);
    }

    // 5. Supervisor Loop: wait for UI process to terminate
    int status;
    waitpid(pid_ui, &status, 0);

    // UI exited; give Core a moment to finish any active cycle then wait for it
    usleep(50000);
    kill(pid_core, SIGTERM);
    waitpid(pid_core, NULL, 0);

    // Core is completely finished; notify Logger that it can shut down
    ipc_log_t shutdown_log;
    memset(&shutdown_log, 0, sizeof(shutdown_log));
    strncpy(shutdown_log.level, "INFO", sizeof(shutdown_log.level) - 1);
    strncpy(shutdown_log.source, "SUPERVISOR", sizeof(shutdown_log.source) - 1);
    strncpy(shutdown_log.message, "__SHUTDOWN__", sizeof(shutdown_log.message) - 1);
    shutdown_log.timestamp = time(NULL);
    mq_send(q3, (const char *)&shutdown_log, sizeof(shutdown_log), 0);

    // Wait for Logger to finish writing to disk and exit
    waitpid(pid_logger, NULL, 0);

    mq_close(q1);
    mq_close(q2);
    mq_close(q3);
    cleanup_queues();

    return 0;
}
