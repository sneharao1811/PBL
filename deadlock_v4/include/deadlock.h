/**
 * @file deadlock.h
 * @brief Core data structures and interface contracts for the Deadlock Simulation System.
 * 
 * OS Project-Based Learning (PBL) - Team 8
 * School of Engineering, St Aloysius (Deemed to be University), Mangaluru
 * 
 * Subsystem Lead: Waqqas (System Integration & Data Models)
 * Contributors: Sneha (UI/CLI/GUI), Siddanth (Banker's Avoidance), Rafath (Detection & Recovery)
 */

#ifndef DEADLOCK_H
#define DEADLOCK_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

#define MAX_PROCESSES 16
#define MAX_RESOURCES 16
#define MAX_NAME_LEN  32

/**
 * @brief System operational state classification.
 */
typedef enum {
    SYSTEM_STATE_SAFE = 0,
    SYSTEM_STATE_UNSAFE,
    SYSTEM_STATE_DEADLOCKED
} system_state_t;

/**
 * @brief Deadlock recovery strategy.
 */
typedef enum {
    RECOVERY_TERMINATE_ALL = 1,
    RECOVERY_TERMINATE_VICTIM_ONE_BY_ONE,
    RECOVERY_PREEMPT_RESOURCES
} recovery_strategy_t;

/**
 * @brief Comprehensive system state holding process allocations, maximum demands,
 * available resource instances, and outstanding requests.
 */
typedef struct {
    int num_processes;
    int num_resources;

    char process_names[MAX_PROCESSES][MAX_NAME_LEN];
    char resource_names[MAX_RESOURCES][MAX_NAME_LEN];

    int total_resources[MAX_RESOURCES];             /* Total instances for each resource type E */
    int available[MAX_RESOURCES];                   /* Available vector A */
    int allocation[MAX_PROCESSES][MAX_RESOURCES];   /* Allocation matrix C */
    int maximum[MAX_PROCESSES][MAX_RESOURCES];      /* Maximum matrix M */
    int need[MAX_PROCESSES][MAX_RESOURCES];         /* Need matrix N = M - C */
    int request[MAX_PROCESSES][MAX_RESOURCES];      /* Current outstanding request matrix Q */

    bool process_active[MAX_PROCESSES];             /* Active / terminated flag */
    int process_priority[MAX_PROCESSES];            /* Priority metric for victim selection */
} deadlock_sys_t;

/**
 * @brief Result packet returned by the Banker's Safety Algorithm.
 */
typedef struct {
    bool is_safe;
    int safe_sequence[MAX_PROCESSES];
    int sequence_len;
    char explanation[512];
} safety_result_t;

/**
 * @brief Result packet returned by the Deadlock Detection Algorithm.
 */
typedef struct {
    bool deadlock_detected;
    bool is_deadlocked[MAX_PROCESSES];
    int deadlocked_processes[MAX_PROCESSES];
    int deadlocked_count;
    char explanation[512];
} detection_result_t;

/**
 * @brief Edge representation for Resource Allocation Graph (RAG).
 */
typedef enum {
    EDGE_ASSIGNMENT = 0, /* Resource -> Process (allocated) */
    EDGE_REQUEST         /* Process -> Resource (requested) */
} rag_edge_type_t;

typedef struct {
    rag_edge_type_t type;
    int process_id;
    int resource_id;
    int instances;
} rag_edge_t;

typedef struct {
    int edge_count;
    rag_edge_t edges[MAX_PROCESSES * MAX_RESOURCES * 2];
    bool has_cycle;
    int cycle_nodes[MAX_PROCESSES + MAX_RESOURCES];
    int cycle_len;
} rag_graph_t;

/* --- Core Initialization and Matrix Operations (Waqqas) --- */
void deadlock_sys_init(deadlock_sys_t *sys);
void deadlock_sys_reset_matrices(deadlock_sys_t *sys);
bool deadlock_sys_recalculate_need(deadlock_sys_t *sys);
bool deadlock_sys_recalculate_available(deadlock_sys_t *sys);
void deadlock_sys_load_textbook_preset(deadlock_sys_t *sys);
void deadlock_sys_load_deadlock_preset(deadlock_sys_t *sys);
void deadlock_sys_load_unsafe_preset(deadlock_sys_t *sys);
system_state_t deadlock_sys_get_state(deadlock_sys_t *sys);
void deadlock_sys_print_status(const deadlock_sys_t *sys);
void deadlock_sys_print_matrices(const deadlock_sys_t *sys);

/* --- Banker's Algorithm & Avoidance (Siddanth) --- */
safety_result_t bankers_check_safety(const deadlock_sys_t *sys);
bool bankers_request_resource(deadlock_sys_t *sys, int process_id, const int *req_vector, char *out_msg, size_t msg_size);
bool bankers_release_resource(deadlock_sys_t *sys, int process_id, const int *rel_vector, char *out_msg, size_t msg_size);

/* --- Deadlock Detection (Rafath) --- */
detection_result_t deadlock_detect(const deadlock_sys_t *sys);

/* --- Deadlock Recovery (Rafath) --- */
int deadlock_select_victim(const deadlock_sys_t *sys, const detection_result_t *det);
bool deadlock_recover(deadlock_sys_t *sys, recovery_strategy_t strategy, char *log_buf, size_t buf_size);

/* --- Resource Allocation Graph (RAG) (Sneha) --- */
void rag_build_graph(const deadlock_sys_t *sys, rag_graph_t *graph);
void rag_print_ascii(const deadlock_sys_t *sys, const rag_graph_t *graph);
void rag_export_dot(const deadlock_sys_t *sys, const rag_graph_t *graph, FILE *fp);
void rag_export_json(const deadlock_sys_t *sys, const rag_graph_t *graph, FILE *fp);

#endif /* DEADLOCK_H */
