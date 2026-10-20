/**
 * @file detection.c
 * @brief Multi-instance Deadlock Detection Algorithm.
 * 
 * Subsystem Lead: Rafath (Diagnostics & Deadlock Detection)
 * OS Project-Based Learning (PBL) - Team 8
 */

#include "../include/deadlock.h"

detection_result_t deadlock_detect(const deadlock_sys_t *sys) {
    detection_result_t res;
    memset(&res, 0, sizeof(detection_result_t));

    if (!sys || sys->num_processes == 0 || sys->num_resources == 0) {
        res.deadlock_detected = false;
        snprintf(res.explanation, sizeof(res.explanation), "Empty system; no processes to evaluate.");
        return res;
    }

    int work[MAX_RESOURCES];
    bool finish[MAX_PROCESSES];

    /* Step 1: Work = Available */
    for (int j = 0; j < sys->num_resources; j++) {
        work[j] = sys->available[j];
    }

    /* Step 2: Initialize Finish */
    for (int i = 0; i < sys->num_processes; i++) {
        if (!sys->process_active[i]) {
            finish[i] = true;
            continue;
        }

        /* If Allocation[i] == 0, Finish[i] = true, else false */
        bool holds_resources = false;
        for (int j = 0; j < sys->num_resources; j++) {
            if (sys->allocation[i][j] > 0) {
                holds_resources = true;
                break;
            }
        }
        finish[i] = !holds_resources;
    }

    /* Step 3: Find index i such that Finish[i] == false and Request[i] <= Work */
    bool progress = true;
    while (progress) {
        progress = false;
        for (int i = 0; i < sys->num_processes; i++) {
            if (!finish[i]) {
                bool can_satisfy = true;
                for (int j = 0; j < sys->num_resources; j++) {
                    if (sys->request[i][j] > work[j]) {
                        can_satisfy = false;
                        break;
                    }
                }

                if (can_satisfy) {
                    /* Process can acquire request, complete, and release its allocations */
                    for (int j = 0; j < sys->num_resources; j++) {
                        work[j] += sys->allocation[i][j];
                    }
                    finish[i] = true;
                    progress = true;
                }
            }
        }
    }

    /* Step 4: Any active process with Finish == false is deadlocked */
    int deadlocked_count = 0;
    char deadlocked_names[256] = "";

    for (int i = 0; i < sys->num_processes; i++) {
        if (sys->process_active[i] && !finish[i]) {
            res.is_deadlocked[i] = true;
            res.deadlocked_processes[deadlocked_count++] = i;

            char temp[16];
            snprintf(temp, sizeof(temp), "%s ", sys->process_names[i]);
            strncat(deadlocked_names, temp, sizeof(deadlocked_names) - strlen(deadlocked_names) - 1);
        }
    }

    res.deadlocked_count = deadlocked_count;
    if (deadlocked_count > 0) {
        res.deadlock_detected = true;
        snprintf(res.explanation, sizeof(res.explanation),
                 "Deadlock detected! Processes [ %s] are in a permanent mutual wait state.",
                 deadlocked_names);
    } else {
        res.deadlock_detected = false;
        snprintf(res.explanation, sizeof(res.explanation),
                 "No deadlock detected. All active processes can acquire their outstanding requests.");
    }

    return res;
}
