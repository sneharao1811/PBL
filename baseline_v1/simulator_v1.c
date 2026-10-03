#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MEM_SIZE 256
#define STACK_SIZE 64
#define LOG_FILE "simulator.log"

// ---- Core State ----
int memory[MEM_SIZE];
int stack[STACK_SIZE];
int sp = -1; // Stack pointer
int pc = 0;  // Program counter (just a counter for commands here)

// ---- Logger Component ----
void logger_log(const char *level, const char *msg) {
    FILE *f = fopen(LOG_FILE, "a");
    if (!f) return;
    
    time_t now;
    time(&now);
    char *date = ctime(&now);
    date[strlen(date) - 1] = '\0'; // Remove newline
    
    fprintf(f, "[%s] [%s] %s\n", date, level, msg);
    fclose(f);
}

// ---- Core Component ----
void core_push(int val) {
    if (sp >= STACK_SIZE - 1) {
        logger_log("ERROR", "Stack Overflow on PUSH");
        printf("Error: Stack Overflow\n");
        return;
    }
    stack[++sp] = val;
    char log_msg[64];
    sprintf(log_msg, "PUSH %d (sp=%d)", val, sp);
    logger_log("INFO", log_msg);
    printf("-> Stack: pushed %d\n", val);
}

void core_pop() {
    if (sp < 0) {
        logger_log("ERROR", "Stack Underflow on POP");
        printf("Error: Stack Underflow\n");
        return;
    }
    int val = stack[sp--];
    char log_msg[64];
    sprintf(log_msg, "POP %d (sp=%d)", val, sp);
    logger_log("INFO", log_msg);
    printf("-> Stack: popped %d\n", val);
}

void core_add() {
    if (sp < 1) {
        logger_log("ERROR", "Not enough elements for ADD");
        printf("Error: Need at least 2 elements on stack to ADD\n");
        return;
    }
    int b = stack[sp--];
    int a = stack[sp--];
    int res = a + b;
    stack[++sp] = res;
    
    char log_msg[64];
    sprintf(log_msg, "ADD %d + %d = %d", a, b, res);
    logger_log("INFO", log_msg);
    printf("-> ALU: %d + %d = %d (pushed to stack)\n", a, b, res);
}

void core_status() {
    printf("=== CPU STATE ===\n");
    printf("Stack Pointer: %d\n", sp);
    if (sp >= 0) {
        printf("Top of Stack: %d\n", stack[sp]);
    }
    printf("=================\n");
    logger_log("INFO", "Status requested");
}

// ---- UI Component ----
void ui_loop() {
    char input[128];
    char cmd[32];
    int arg;

    printf("Welcome to the Single-Process Simulator!\n");
    printf("Available commands: PUSH <val>, POP, ADD, STATUS, EXIT\n");
    
    logger_log("INFO", "Simulator Started");

    while (1) {
        printf("\nSIM> ");
        if (!fgets(input, sizeof(input), stdin)) break;
        
        input[strcspn(input, "\n")] = '\0';
        
        if (strlen(input) == 0) continue;

        if (sscanf(input, "%s %d", cmd, &arg) >= 1) {
            pc++;
            if (strcmp(cmd, "EXIT") == 0) {
                logger_log("INFO", "Simulator Shutdown via EXIT");
                printf("Goodbye!\n");
                break;
            } else if (strcmp(cmd, "PUSH") == 0) {
                core_push(arg);
            } else if (strcmp(cmd, "POP") == 0) {
                core_pop();
            } else if (strcmp(cmd, "ADD") == 0) {
                core_add();
            } else if (strcmp(cmd, "STATUS") == 0) {
                core_status();
            } else {
                logger_log("ERROR", "Unknown command entered");
                printf("Unknown command: %s\n", cmd);
            }
        }
    }
}

int main(void) {
    // Clear log file on startup
    FILE *f = fopen(LOG_FILE, "w");
    if (f) fclose(f);
    
    ui_loop();
    return 0;
}
