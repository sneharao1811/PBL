/**
 * @file banker.c
 * @brief Banker's Algorithm implementation for Deadlock Avoidance.
 * 
 * Subsystem Lead: Siddanth (Banker's Avoidance & Algorithmic Engine)
 * OS Project-Based Learning (PBL) - Team 8
 */

#include "../include/deadlock.h"

safety_result_t bankers_check_safety(const deadlock_sys_t *sys) {
    safety_result_t res;
    memset(&res, 0, sizeof(safety_result_t));
    if (!sys || sys->num_processes == 0 || sys->num_resources == 0) {
        res.is_safe = false;
        snprintf(res.explanation, sizeof(res.explanation), "Empty system or zero resources.");
        return res;
    }

    int work[MAX_RESOURCES];
    bool finish[MAX_PROCESSES];

    /* Step 1: Initialize Work = Available, Finish = false for active processes */
    for (int j = 0; j < sys->num_resources; j++) {
        work[j] = sys->available[j];
    }
    for (int i = 0; i < sys->num_processes; i++) {
        finish[i] = !sys->process_active[i];
    }

    /* Step 2 & 3: Find index i such that Finish[i] == false and Need[i] <= Work */
    int count = 0;
    int active_process_count = 0;
    for (int i = 0; i < sys->num_processes; i++) {
        if (sys->process_active[i]) active_process_count++;
    }

    bool progress = true;
    while (progress && count < active_process_count) {
        progress = false;
        for (int i = 0; i < sys->num_processes; i++) {
            if (!finish[i]) {
                bool can_satisfy = true;
                for (int j = 0; j < sys->num_resources; j++) {
                    if (sys->need[i][j] > work[j]) {
                        can_satisfy = false;
                        break;
                    }
                }

                if (can_satisfy) {
                    /* Process can execute to completion, then release allocated resources */
                    for (int j = 0; j < sys->num_resources; j++) {
                        work[j] += sys->allocation[i][j];
                    }
                    finish[i] = true;
                    res.safe_sequence[count++] = i;
                    progress = true;
                }
            }
        }
    }

    res.sequence_len = count;

    /* Step 4: If all finish == true, system is in a safe state */
    if (count == active_process_count) {
        res.is_safe = true;
        snprintf(res.explanation, sizeof(res.explanation),
                 "A valid execution order exists where every process can acquire maximum resources and terminate.");
    } else {
        res.is_safe = false;
        char blocked[128] = "";
        for (int i = 0; i < sys->num_processes; i++) {
            if (!finish[i]) {
                char temp[16];
                snprintf(temp, sizeof(temp), "%s ", sys->process_names[i]);
                strncat(blocked, temp, sizeof(blocked) - strlen(blocked) - 1);
            }
        }
        snprintf(res.explanation, sizeof(res.explanation),
                 "No safe sequence found. Unfulfillable demands for processes: [ %s]. System is in an UNSAFE state.",
                 blocked);
    }

    return res;
}

bool bankers_request_resource(deadlock_sys_t *sys, int process_id, const int *req_vector, char *out_msg, size_t msg_size) {
    if (!sys || !req_vector) {
        if (out_msg) snprintf(out_msg, msg_size, "Invalid arguments passed to request handler.");
        return false;
    }

    if (process_id < 0 || process_id >= sys->num_processes || !sys->process_active[process_id]) {
        if (out_msg) snprintf(out_msg, msg_size, "Process index %d is invalid or terminated.", process_id);
        return false;
    }

    /* Condition 1: Request <= Need */
    for (int j = 0; j < sys->num_resources; j++) {
        if (req_vector[j] > sys->need[process_id][j]) {
            if (out_msg) {
                snprintf(out_msg, msg_size,
                         "DENIED: %s requested %d of %s, which exceeds declared need (%d).",
                         sys->process_names[process_id], req_vector[j], sys->resource_names[j],
                         sys->need[process_id][j]);
            }
            return false;
        }
    }

    /* Condition 2: Request <= Available */
    for (int j = 0; j < sys->num_resources; j++) {
        if (req_vector[j] > sys->available[j]) {
            /* Record as pending request */
            sys->request[process_id][j] = req_vector[j];
            if (out_msg) {
                snprintf(out_msg, msg_size,
                         "BLOCKED: %s requested %d of %s, but only %d currently available. Process must wait.",
                         sys->process_names[process_id], req_vector[j], sys->resource_names[j],
                         sys->available[j]);
            }
            return false;
        }
    }

    /* Step 3: Hypothetically allocate resources */
    deadlock_sys_t temp_sys;
    memcpy(&temp_sys, sys, sizeof(deadlock_sys_t));

    for (int j = 0; j < sys->num_resources; j++) {
        temp_sys.available[j] -= req_vector[j];
        temp_sys.allocation[process_id][j] += req_vector[j];
        temp_sys.need[process_id][j] -= req_vector[j];
        temp_sys.request[process_id][j] = 0;
    }

    /* Step 4: Evaluate safety of hypothetical state */
    safety_result_t safe = bankers_check_safety(&temp_sys);
    if (safe.is_safe) {
        /* Safe! Commit the allocation to real system */
        for (int j = 0; j < sys->num_resources; j++) {
            sys->available[j] -= req_vector[j];
            sys->allocation[process_id][j] += req_vector[j];
            sys->need[process_id][j] -= req_vector[j];
            sys->request[process_id][j] = 0;
        }
        if (out_msg) {
            snprintf(out_msg, msg_size,
                     "GRANTED: Request approved. System remains SAFE with new safe sequence.");
        }
        return true;
    } else {
        /* Unsafe: Rollback and reject */
        sys->request[process_id][0] = req_vector[0]; // Mark request pending
        for (int j = 1; j < sys->num_resources; j++) {
            sys->request[process_id][j] = req_vector[j];
        }
        if (out_msg) {
            snprintf(out_msg, msg_size,
                     "DENIED BY BANKER'S AVOIDANCE: Granting this request leads to an UNSAFE state (potential deadlock). Request queued.");
        }
        return false;
    }
}

bool bankers_release_resource(deadlock_sys_t *sys, int process_id, const int *rel_vector, char *out_msg, size_t msg_size) {
    if (!sys || !rel_vector || process_id < 0 || process_id >= sys->num_processes) {
        if (out_msg) snprintf(out_msg, msg_size, "Invalid arguments for resource release.");
        return false;
    }

    for (int j = 0; j < sys->num_resources; j++) {
        if (rel_vector[j] > sys->allocation[process_id][j]) {
            if (out_msg) {
                snprintf(out_msg, msg_size,
                         "ERROR: %s cannot release %d of %s (currently only holds %d).",
                         sys->process_names[process_id], rel_vector[j], sys->resource_names[j],
                         sys->allocation[process_id][j]);
            }
            return false;
        }
    }

    for (int j = 0; j < sys->num_resources; j++) {
        sys->allocation[process_id][j] -= rel_vector[j];
        sys->available[j] += rel_vector[j];
        sys->need[process_id][j] += rel_vector[j];
    }

    if (out_msg) {
        snprintf(out_msg, msg_size, "SUCCESS: %s released requested resources back to Available pool.",
                 sys->process_names[process_id]);
    }
    return true;
}
