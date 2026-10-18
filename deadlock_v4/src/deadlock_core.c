/**
 * @file deadlock_core.c
 * @brief Core state management, matrix math, presets, and terminal displays.
 * 
 * Lead: Waqqas (System Integration & Data Models)
 * OS Project-Based Learning (PBL) - Team 8
 */

#include "../include/deadlock.h"

void deadlock_sys_init(deadlock_sys_t *sys) {
    if (!sys) return;
    memset(sys, 0, sizeof(deadlock_sys_t));
    sys->num_processes = 0;
    sys->num_resources = 0;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        sys->process_active[i] = true;
        sys->process_priority[i] = i + 1;
        snprintf(sys->process_names[i], MAX_NAME_LEN, "P%d", i);
    }
    for (int j = 0; j < MAX_RESOURCES; j++) {
        snprintf(sys->resource_names[j], MAX_NAME_LEN, "R%d", j);
    }
}

void deadlock_sys_reset_matrices(deadlock_sys_t *sys) {
    if (!sys) return;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        sys->process_active[i] = true;
        for (int j = 0; j < MAX_RESOURCES; j++) {
            sys->allocation[i][j] = 0;
            sys->maximum[i][j] = 0;
            sys->need[i][j] = 0;
            sys->request[i][j] = 0;
        }
    }
    for (int j = 0; j < MAX_RESOURCES; j++) {
        sys->available[j] = sys->total_resources[j];
    }
}

bool deadlock_sys_recalculate_need(deadlock_sys_t *sys) {
    if (!sys) return false;
    bool valid = true;
    for (int i = 0; i < sys->num_processes; i++) {
        if (!sys->process_active[i]) {
            for (int j = 0; j < sys->num_resources; j++) {
                sys->need[i][j] = 0;
            }
            continue;
        }
        for (int j = 0; j < sys->num_resources; j++) {
            if (sys->allocation[i][j] > sys->maximum[i][j]) {
                valid = false;
            }
            sys->need[i][j] = sys->maximum[i][j] - sys->allocation[i][j];
            if (sys->need[i][j] < 0) {
                sys->need[i][j] = 0;
            }
        }
    }
    return valid;
}

bool deadlock_sys_recalculate_available(deadlock_sys_t *sys) {
    if (!sys) return false;
    for (int j = 0; j < sys->num_resources; j++) {
        int allocated_sum = 0;
        for (int i = 0; i < sys->num_processes; i++) {
            if (sys->process_active[i]) {
                allocated_sum += sys->allocation[i][j];
            }
        }
        sys->available[j] = sys->total_resources[j] - allocated_sum;
        if (sys->available[j] < 0) {
            sys->available[j] = 0;
        }
    }
    return true;
}

void deadlock_sys_load_textbook_preset(deadlock_sys_t *sys) {
    deadlock_sys_init(sys);
    sys->num_processes = 5;
    sys->num_resources = 3;

    snprintf(sys->resource_names[0], MAX_NAME_LEN, "A (CPU)");
    snprintf(sys->resource_names[1], MAX_NAME_LEN, "B (RAM)");
    snprintf(sys->resource_names[2], MAX_NAME_LEN, "C (Disk)");

    sys->total_resources[0] = 10;
    sys->total_resources[1] = 5;
    sys->total_resources[2] = 7;

    /* Allocation Matrix */
    int alloc[5][3] = {
        {0, 1, 0}, /* P0 */
        {2, 0, 0}, /* P1 */
        {3, 0, 2}, /* P2 */
        {2, 1, 1}, /* P3 */
        {0, 0, 2}  /* P4 */
    };

    /* Maximum Matrix */
    int max_m[5][3] = {
        {7, 5, 3}, /* P0 */
        {3, 2, 2}, /* P1 */
        {9, 0, 2}, /* P2 */
        {2, 2, 2}, /* P3 */
        {4, 3, 3}  /* P4 */
    };

    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 3; j++) {
            sys->allocation[i][j] = alloc[i][j];
            sys->maximum[i][j] = max_m[i][j];
        }
    }

    deadlock_sys_recalculate_need(sys);
    deadlock_sys_recalculate_available(sys);
}

void deadlock_sys_load_deadlock_preset(deadlock_sys_t *sys) {
    deadlock_sys_init(sys);
    sys->num_processes = 3;
    sys->num_resources = 3;

    snprintf(sys->resource_names[0], MAX_NAME_LEN, "R1 (Mutex)");
    snprintf(sys->resource_names[1], MAX_NAME_LEN, "R2 (Tape)");
    snprintf(sys->resource_names[2], MAX_NAME_LEN, "R3 (Plotter)");

    sys->total_resources[0] = 1;
    sys->total_resources[1] = 1;
    sys->total_resources[2] = 1;

    /* Circular wait: P0 holds R0, requests R1; P1 holds R1, requests R2; P2 holds R2, requests R0 */
    sys->allocation[0][0] = 1; sys->allocation[0][1] = 0; sys->allocation[0][2] = 0;
    sys->allocation[1][0] = 0; sys->allocation[1][1] = 1; sys->allocation[1][2] = 0;
    sys->allocation[2][0] = 0; sys->allocation[2][1] = 0; sys->allocation[2][2] = 1;

    sys->maximum[0][0] = 1; sys->maximum[0][1] = 1; sys->maximum[0][2] = 0;
    sys->maximum[1][0] = 0; sys->maximum[1][1] = 1; sys->maximum[1][2] = 1;
    sys->maximum[2][0] = 1; sys->maximum[2][1] = 0; sys->maximum[2][2] = 1;

    sys->request[0][1] = 1;
    sys->request[1][2] = 1;
    sys->request[2][0] = 1;

    deadlock_sys_recalculate_need(sys);
    deadlock_sys_recalculate_available(sys);
}

void deadlock_sys_load_unsafe_preset(deadlock_sys_t *sys) {
    deadlock_sys_init(sys);
    sys->num_processes = 4;
    sys->num_resources = 2;

    snprintf(sys->resource_names[0], MAX_NAME_LEN, "GPU");
    snprintf(sys->resource_names[1], MAX_NAME_LEN, "Network");

    sys->total_resources[0] = 6;
    sys->total_resources[1] = 6;

    int alloc[4][2] = {
        {2, 1},
        {1, 2},
        {2, 1},
        {1, 1}
    };

    int max_m[4][2] = {
        {4, 3},
        {3, 4},
        {4, 3},
        {3, 3}
    };

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 2; j++) {
            sys->allocation[i][j] = alloc[i][j];
            sys->maximum[i][j] = max_m[i][j];
        }
    }

    deadlock_sys_recalculate_need(sys);
    deadlock_sys_recalculate_available(sys);
}

system_state_t deadlock_sys_get_state(deadlock_sys_t *sys) {
    detection_result_t det = deadlock_detect(sys);
    if (det.deadlock_detected) {
        return SYSTEM_STATE_DEADLOCKED;
    }
    safety_result_t safe = bankers_check_safety(sys);
    if (safe.is_safe) {
        return SYSTEM_STATE_SAFE;
    }
    return SYSTEM_STATE_UNSAFE;
}

void deadlock_sys_print_matrices(const deadlock_sys_t *sys) {
    if (!sys || sys->num_processes == 0 || sys->num_resources == 0) {
        printf("\n[Notice] System is empty. Please create processes and resources first.\n");
        return;
    }

    printf("\n========================================================================\n");
    printf("                  SYSTEM MATRICES & RESOURCE VECTORS                    \n");
    printf("========================================================================\n");

    printf("Total Resources Vector (E):\n  ");
    for (int j = 0; j < sys->num_resources; j++) {
        printf("%s: %-3d  ", sys->resource_names[j], sys->total_resources[j]);
    }
    printf("\nAvailable Resource Vector (A):\n  ");
    for (int j = 0; j < sys->num_resources; j++) {
        printf("%s: %-3d  ", sys->resource_names[j], sys->available[j]);
    }
    printf("\n------------------------------------------------------------------------\n");

    printf("%-8s | ", "Process");
    printf("Allocation (C)   | ");
    printf("Maximum (M)      | ");
    printf("Need (N = M - C) | ");
    printf("Outstanding Req  | Status\n");

    printf("---------+------------------+------------------+------------------+------------------+----------\n");

    for (int i = 0; i < sys->num_processes; i++) {
        printf("%-8s | ", sys->process_names[i]);

        /* Allocation */
        for (int j = 0; j < sys->num_resources; j++) {
            printf("%d ", sys->allocation[i][j]);
        }
        printf("%*s | ", (int)(16 - sys->num_resources * 2), "");

        /* Maximum */
        for (int j = 0; j < sys->num_resources; j++) {
            printf("%d ", sys->maximum[i][j]);
        }
        printf("%*s | ", (int)(16 - sys->num_resources * 2), "");

        /* Need */
        for (int j = 0; j < sys->num_resources; j++) {
            printf("%d ", sys->need[i][j]);
        }
        printf("%*s | ", (int)(16 - sys->num_resources * 2), "");

        /* Outstanding Request */
        for (int j = 0; j < sys->num_resources; j++) {
            printf("%d ", sys->request[i][j]);
        }
        printf("%*s | ", (int)(16 - sys->num_resources * 2), "");

        printf("%s\n", sys->process_active[i] ? "ACTIVE" : "TERMINATED");
    }
    printf("========================================================================\n\n");
}

void deadlock_sys_print_status(const deadlock_sys_t *sys) {
    if (!sys || sys->num_processes == 0) {
        printf("\n[Notice] System is empty.\n");
        return;
    }

    safety_result_t safe = bankers_check_safety(sys);
    detection_result_t det = deadlock_detect(sys);

    printf("\n========================================================================\n");
    printf("                         SYSTEM STATUS TELEMETRY                        \n");
    printf("========================================================================\n");
    printf("Total Processes : %d active, %d total\n", sys->num_processes, sys->num_processes);
    printf("Resource Types  : %d\n", sys->num_resources);

    if (det.deadlock_detected) {
        printf("State Indicator : \033[1;31m[ DEADLOCKED ]\033[0m\n");
        printf("Deadlocked PIDs : ");
        for (int k = 0; k < det.deadlocked_count; k++) {
            printf("%s ", sys->process_names[det.deadlocked_processes[k]]);
        }
        printf("\nDiagnosis       : %s\n", det.explanation);
    } else if (safe.is_safe) {
        printf("State Indicator : \033[1;32m[ SAFE ]\033[0m\n");
        printf("Safe Sequence   : < ");
        for (int k = 0; k < safe.sequence_len; k++) {
            printf("%s%s", sys->process_names[safe.safe_sequence[k]],
                   (k == safe.sequence_len - 1) ? "" : ", ");
        }
        printf(" >\n");
        printf("Diagnosis       : %s\n", safe.explanation);
    } else {
        printf("State Indicator : \033[1;33m[ UNSAFE ]\033[0m\n");
        printf("Diagnosis       : %s\n", safe.explanation);
    }

    /* Resource Utilization */
    printf("\nResource Utilization Breakdown:\n");
    for (int j = 0; j < sys->num_resources; j++) {
        int in_use = sys->total_resources[j] - sys->available[j];
        double pct = (sys->total_resources[j] > 0) ? ((double)in_use / sys->total_resources[j]) * 100.0 : 0.0;
        printf("  * %-14s: Total=%-2d | Allocated=%-2d | Free=%-2d | Utilized=%5.1f%%\n",
               sys->resource_names[j], sys->total_resources[j], in_use, sys->available[j], pct);
    }
    printf("========================================================================\n\n");
}
