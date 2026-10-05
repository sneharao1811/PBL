#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>
#include <sys/types.h>
#include <time.h>

// Queue names for POSIX Message Queues
#define QUEUE_UI_TO_CORE   "/pbl8_ui_to_core"
#define QUEUE_CORE_TO_UI   "/pbl8_core_to_ui"
#define QUEUE_TO_LOGGER    "/pbl8_to_logger"

#define MAX_QUEUE_MSGS     10
#define MAX_LOG_TEXT       256
#define MAX_DISPLAY_TEXT   256

// Architectural Limits
#define MEM_SIZE           256
#define STACK_SIZE         64
#define LOG_FILE_PATH      "simulator.log"

// Command Opcodes
typedef enum {
    CMD_PUSH = 1,
    CMD_POP,
    CMD_ADD,
    CMD_STATUS,
    CMD_EXIT
} opcode_t;

// Execution Status Codes
typedef enum {
    STATUS_OK = 0,
    STATUS_ERR_OVERFLOW,
    STATUS_ERR_UNDERFLOW,
    STATUS_ERR_OPERANDS,
    STATUS_ERR_UNKNOWN,
    STATUS_SHUTDOWN
} status_t;

// Request packet: UI -> Core
typedef struct {
    uint32_t req_id;
    pid_t sender_pid;
    opcode_t opcode;
    int operand;
} ipc_req_t;

// Response packet: Core -> UI
typedef struct {
    uint32_t req_id;
    status_t status;
    int sp;
    int top_val;
    int result;
    char display_msg[MAX_DISPLAY_TEXT];
} ipc_resp_t;

// Log packet: Any Process -> Logger Process
typedef struct {
    char level[16];                // "INFO", "WARN", "ERROR"
    char source[32];               // "UI", "CORE", "SUPERVISOR"
    char message[MAX_LOG_TEXT];
    time_t timestamp;
} ipc_log_t;

#endif // PROTOCOL_H
