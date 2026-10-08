#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <mqueue.h>
#include <pthread.h>
#include "../include/protocol.h"

// ---- Internal Core State ----
static int memory[MEM_SIZE];
static int stack[STACK_SIZE];
static int sp = -1;
static int pc = 0;

// Instruction Queue (Internal Queue subsystem as required)
#define INST_QUEUE_CAPACITY 32
static ipc_req_t inst_queue[INST_QUEUE_CAPACITY];
static int queue_head = 0;
static int queue_tail = 0;
static int queue_count = 0;

static volatile sig_atomic_t keep_running = 1;
static mqd_t mq_ui2core = (mqd_t)-1;
static mqd_t mq_core2ui = (mqd_t)-1;
static mqd_t mq_logger  = (mqd_t)-1;

// ---- Asynchronous Logging Thread Subsystem ----
#define LOG_RING_SIZE 64
static ipc_log_t log_ring[LOG_RING_SIZE];
static int ring_head = 0;
static int ring_tail = 0;
static int ring_count = 0;
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  log_cond  = PTHREAD_COND_INITIALIZER;
static pthread_t log_thread;

static void queue_async_log(const char *level, const char *msg) {
    pthread_mutex_lock(&log_mutex);
    if (ring_count < LOG_RING_SIZE) {
        ipc_log_t *item = &log_ring[ring_tail];
        strncpy(item->level, level, sizeof(item->level) - 1);
        strncpy(item->source, "CORE", sizeof(item->source) - 1);
        strncpy(item->message, msg, sizeof(item->message) - 1);
        item->timestamp = time(NULL);
        ring_tail = (ring_tail + 1) % LOG_RING_SIZE;
        ring_count++;
        pthread_cond_signal(&log_cond);
    }
    pthread_mutex_unlock(&log_mutex);
}

static void *logger_worker_thread(void *arg) {
    (void)arg;
    while (keep_running || ring_count > 0) {
        pthread_mutex_lock(&log_mutex);
        while (ring_count == 0 && keep_running) {
            pthread_cond_wait(&log_cond, &log_mutex);
        }
        if (ring_count == 0 && !keep_running) {
            pthread_mutex_unlock(&log_mutex);
            break;
        }
        ipc_log_t item = log_ring[ring_head];
        ring_head = (ring_head + 1) % LOG_RING_SIZE;
        ring_count--;
        pthread_mutex_unlock(&log_mutex);

        if (mq_logger != (mqd_t)-1) {
            struct timespec ts;
            clock_gettime(CLOCK_REALTIME, &ts);
            ts.tv_nsec += 20000000; // 20ms timeout
            if (ts.tv_nsec >= 1000000000) {
                ts.tv_sec += 1;
                ts.tv_nsec -= 1000000000;
            }
            mq_timedsend(mq_logger, (const char *)&item, sizeof(ipc_log_t), 0, &ts);
        }
    }
    return NULL;
}

// ---- Queue Operations ----
static int enqueue_instruction(const ipc_req_t *req) {
    if (queue_count >= INST_QUEUE_CAPACITY) return -1;
    inst_queue[queue_tail] = *req;
    queue_tail = (queue_tail + 1) % INST_QUEUE_CAPACITY;
    queue_count++;
    return 0;
}

static int dequeue_instruction(ipc_req_t *req) {
    if (queue_count == 0) return -1;
    *req = inst_queue[queue_head];
    queue_head = (queue_head + 1) % INST_QUEUE_CAPACITY;
    queue_count--;
    return 0;
}

// ---- CPU Core Operations ----
static void execute_request(const ipc_req_t *req, ipc_resp_t *resp) {
    resp->req_id = req->req_id;
    resp->status = STATUS_OK;
    resp->sp = sp;
    resp->top_val = (sp >= 0) ? stack[sp] : 0;
    resp->result = 0;
    resp->display_msg[0] = '\0';

    char log_buf[128];
    pc++;

    switch (req->opcode) {
        case CMD_PUSH: {
            if (sp >= STACK_SIZE - 1) {
                resp->status = STATUS_ERR_OVERFLOW;
                snprintf(resp->display_msg, sizeof(resp->display_msg), "Error: Stack Overflow");
                queue_async_log("ERROR", "Stack Overflow on PUSH");
            } else {
                stack[++sp] = req->operand;
                resp->sp = sp;
                resp->top_val = req->operand;
                snprintf(resp->display_msg, sizeof(resp->display_msg), "-> Stack: pushed %d (sp=%d)", req->operand, sp);
                snprintf(log_buf, sizeof(log_buf), "PUSH %d (sp=%d)", req->operand, sp);
                queue_async_log("INFO", log_buf);
            }
            break;
        }
        case CMD_POP: {
            if (sp < 0) {
                resp->status = STATUS_ERR_UNDERFLOW;
                snprintf(resp->display_msg, sizeof(resp->display_msg), "Error: Stack Underflow");
                queue_async_log("ERROR", "Stack Underflow on POP");
            } else {
                int val = stack[sp--];
                resp->sp = sp;
                resp->top_val = (sp >= 0) ? stack[sp] : 0;
                resp->result = val;
                snprintf(resp->display_msg, sizeof(resp->display_msg), "-> Stack: popped %d (sp=%d)", val, sp);
                snprintf(log_buf, sizeof(log_buf), "POP %d (sp=%d)", val, sp);
                queue_async_log("INFO", log_buf);
            }
            break;
        }
        case CMD_ADD: {
            if (sp < 1) {
                resp->status = STATUS_ERR_OPERANDS;
                snprintf(resp->display_msg, sizeof(resp->display_msg), "Error: Need at least 2 elements on stack to ADD");
                queue_async_log("ERROR", "Not enough elements for ADD");
            } else {
                int b = stack[sp--];
                int a = stack[sp--];
                int res = a + b;
                stack[++sp] = res;
                resp->sp = sp;
                resp->top_val = res;
                resp->result = res;
                snprintf(resp->display_msg, sizeof(resp->display_msg), "-> ALU: %d + %d = %d (pushed to stack)", a, b, res);
                snprintf(log_buf, sizeof(log_buf), "ADD %d + %d = %d", a, b, res);
                queue_async_log("INFO", log_buf);
            }
            break;
        }
        case CMD_STATUS: {
            resp->status = STATUS_OK;
            resp->sp = sp;
            resp->top_val = (sp >= 0) ? stack[sp] : 0;
            snprintf(resp->display_msg, sizeof(resp->display_msg),
                     "=== CPU STATE ===\nStack Pointer: %d\nTop of Stack: %s%d\nInstruction Count: %d\nQueue Pending: %d\n=================",
                     sp, (sp >= 0 ? "" : "(empty) "), (sp >= 0 ? stack[sp] : 0), pc, queue_count);
            queue_async_log("INFO", "Status requested and dispatched");
            break;
        }
        case CMD_EXIT: {
            resp->status = STATUS_SHUTDOWN;
            snprintf(resp->display_msg, sizeof(resp->display_msg), "Core acknowledged shutdown.");
            queue_async_log("INFO", "Core received EXIT command");
            keep_running = 0;
            break;
        }
        default: {
            resp->status = STATUS_ERR_UNKNOWN;
            snprintf(resp->display_msg, sizeof(resp->display_msg), "Error: Unknown OpCode received");
            queue_async_log("ERROR", "Unknown OpCode received");
            break;
        }
    }
}

static void handle_signal(int sig) {
    (void)sig;
    keep_running = 0;
    pthread_mutex_lock(&log_mutex);
    pthread_cond_broadcast(&log_cond);
    pthread_mutex_unlock(&log_mutex);
}

int main(void) {
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    memset(memory, 0, sizeof(memory));

    // Open existing message queues created by supervisor
    mq_ui2core = mq_open(QUEUE_UI_TO_CORE, O_RDONLY);
    if (mq_ui2core == (mqd_t)-1) {
        perror("[Core] Failed to open QUEUE_UI_TO_CORE");
        return 1;
    }

    mq_core2ui = mq_open(QUEUE_CORE_TO_UI, O_WRONLY);
    if (mq_core2ui == (mqd_t)-1) {
        perror("[Core] Failed to open QUEUE_CORE_TO_UI");
        mq_close(mq_ui2core);
        return 1;
    }

    mq_logger = mq_open(QUEUE_TO_LOGGER, O_WRONLY);
    if (mq_logger == (mqd_t)-1) {
        perror("[Core] Warning: Failed to open QUEUE_TO_LOGGER");
    }

    // Launch asynchronous logging worker thread
    pthread_create(&log_thread, NULL, logger_worker_thread, NULL);
    queue_async_log("INFO", "Core Engine Process Initialized");

    ipc_req_t req;
    ipc_resp_t resp;

    while (keep_running) {
        ssize_t bytes_read = mq_receive(mq_ui2core, (char *)&req, sizeof(ipc_req_t), NULL);
        if (bytes_read >= 0) {
            enqueue_instruction(&req);
            ipc_req_t to_run;
            if (dequeue_instruction(&to_run) == 0) {
                execute_request(&to_run, &resp);
                mq_send(mq_core2ui, (const char *)&resp, sizeof(ipc_resp_t), 0);
            }
        } else {
            if (!keep_running) break;
        }
    }

    queue_async_log("INFO", "Core Engine Process Terminating");
    
    // Stop thread and drain logs
    pthread_mutex_lock(&log_mutex);
    pthread_cond_signal(&log_cond);
    pthread_mutex_unlock(&log_mutex);
    pthread_join(log_thread, NULL);

    mq_close(mq_ui2core);
    mq_close(mq_core2ui);
    if (mq_logger != (mqd_t)-1) {
        mq_close(mq_logger);
    }

    return 0;
}
