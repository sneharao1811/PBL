/**
 * @file recovery.c
 * @brief Deadlock Recovery Strategies (Process Termination & Resource Preemption).
 * 
 * Subsystem Lead: Rafath (Diagnostics & Telemetry Lead)
 * OS Project-Based Learning (PBL) - Team 8
 */

#include "../include/deadlock.h"

int deadlock_select_victim(const deadlock_sys_t *sys, const detection_result_t *det) {
    if (!sys || !det || det->deadlocked_count == 0) return -1;

    int best_victim = -1;
    int max_resources_held = -1;

    /* Cost heuristic: select process holding the largest amount of total allocated resources */
    for (int k = 0; k < det->deadlocked_count; k++) {
        int pid = det->deadlocked_processes[k];
        if (!sys->process_active[pid]) continue;

        int total_held = 0;
        for (int j = 0; j < sys->num_resources; j++) {
            total_held += sys->allocation[pid][j];
        }

        if (total_held > max_resources_held) {
            max_resources_held = total_held;
            best_victim = pid;
        }
    }

    return best_victim;
}

bool deadlock_recover(deadlock_sys_t *sys, recovery_strategy_t strategy, char *log_buf, size_t buf_size) {
    if (!sys) {
        if (log_buf) snprintf(log_buf, buf_size, "Invalid system pointer.");
        return false;
    }

    detection_result_t det = deadlock_detect(sys);
    if (!det.deadlock_detected) {
        if (log_buf) snprintf(log_buf, buf_size, "No deadlock present; recovery not required.");
        return true;
    }

    if (log_buf) log_buf[0] = '\0';

    if (strategy == RECOVERY_TERMINATE_ALL) {
        /* Strategy 1: Abort all deadlocked processes simultaneously */
        for (int k = 0; k < det.deadlocked_count; k++) {
            int pid = det.deadlocked_processes[k];
            for (int j = 0; j < sys->num_resources; j++) {
                sys->available[j] += sys->allocation[pid][j];
                sys->allocation[pid][j] = 0;
                sys->need[pid][j] = 0;
                sys->request[pid][j] = 0;
            }
            sys->process_active[pid] = false;
        }

        if (log_buf) {
            snprintf(log_buf, buf_size,
                     "RECOVERY SUCCESS: Aborted %d deadlocked processes. All held resources reclaimed.",
                     det.deadlocked_count);
        }
        return true;
    }
    else if (strategy == RECOVERY_TERMINATE_VICTIM_ONE_BY_ONE) {
        /* Strategy 2: Abort one process at a time until the deadlock cycle is broken */
        int terminated_count = 0;
        char terminated_names[256] = "";

        while (det.deadlock_detected) {
            int victim = deadlock_select_victim(sys, &det);
            if (victim < 0) break;

            /* Terminate victim and reclaim resources */
            for (int j = 0; j < sys->num_resources; j++) {
                sys->available[j] += sys->allocation[victim][j];
                sys->allocation[victim][j] = 0;
                sys->need[victim][j] = 0;
                sys->request[victim][j] = 0;
            }
            sys->process_active[victim] = false;
            terminated_count++;

            char temp[16];
            snprintf(temp, sizeof(temp), "%s ", sys->process_names[victim]);
            strncat(terminated_names, temp, sizeof(terminated_names) - strlen(terminated_names) - 1);

            /* Re-evaluate system state */
            det = deadlock_detect(sys);
        }

        if (log_buf) {
            snprintf(log_buf, buf_size,
                     "RECOVERY SUCCESS: Selectively aborted %d victim process(es): [ %s]. System is no longer deadlocked.",
                     terminated_count, terminated_names);
        }
        return true;
    }
    else if (strategy == RECOVERY_PREEMPT_RESOURCES) {
        /* Strategy 3: Preempt resources from victim, roll back request */
        int victim = deadlock_select_victim(sys, &det);
        if (victim < 0) return false;

        for (int j = 0; j < sys->num_resources; j++) {
            sys->available[j] += sys->allocation[victim][j];
            sys->need[victim][j] += sys->allocation[victim][j];
            sys->allocation[victim][j] = 0;
            sys->request[victim][j] = 0;
        }

        if (log_buf) {
            snprintf(log_buf, buf_size,
                     "RECOVERY (PREEMPTION): Preempted all allocated resources from victim %s and rolled back state.",
                     sys->process_names[victim]);
        }
        return true;
    }

    return false;
}
