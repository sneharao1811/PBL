#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <mqueue.h>
#include "../include/protocol.h"

static volatile sig_atomic_t keep_running = 1;

static void handle_signal(int sig) {
    (void)sig;
    keep_running = 0;
}

int main(void) {
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_QUEUE_MSGS;
    attr.mq_msgsize = sizeof(ipc_log_t);
    attr.mq_curmsgs = 0;

    mqd_t mq_log = mq_open(QUEUE_TO_LOGGER, O_CREAT | O_RDONLY, 0666, &attr);
    if (mq_log == (mqd_t)-1) {
        perror("[Logger] mq_open failed");
        return 1;
    }

    FILE *f = fopen(LOG_FILE_PATH, "a");
    if (!f) {
        perror("[Logger] Failed to open log file");
        mq_close(mq_log);
        return 1;
    }
    setvbuf(f, NULL, _IOLBF, 0);

    // Flush on startup
    fprintf(f, "=== LOGGER DAEMON STARTED (PID %d) ===\n", getpid());

    ipc_log_t log_msg;
    while (keep_running) {
        ssize_t bytes_read = mq_receive(mq_log, (char *)&log_msg, sizeof(ipc_log_t), NULL);
        if (bytes_read >= 0) {
            // Check for termination marker
            if (strcmp(log_msg.message, "__SHUTDOWN__") == 0) {
                break;
            }

            char time_buf[64];
            struct tm *tm_info = localtime(&log_msg.timestamp);
            if (tm_info) {
                strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", tm_info);
            } else {
                snprintf(time_buf, sizeof(time_buf), "UNKNOWN_TIME");
            }

            fprintf(f, "[%s] [%s] [%s] %s\n", time_buf, log_msg.source, log_msg.level, log_msg.message);
        } else {
            if (!keep_running) break;
        }
    }

    fprintf(f, "=== LOGGER DAEMON SHUTDOWN (PID %d) ===\n", getpid());
    fflush(f);
    fclose(f);
    mq_close(mq_log);

    return 0;
}
