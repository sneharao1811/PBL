/**
 * @file lru.c
 * @brief Least Recently Used (LRU) Page Replacement Algorithm
 * 
 * Implements LRU cache replacement using an array of reference timestamps.
 * Updates the access timestamp whenever a page is hit or newly loaded.
 * On page fault under full capacity, evicts the page with the minimum timestamp.
 * Adheres to the stack property, guaranteeing immunity to Belady's Anomaly.
 * 
 * Team 8 - Systems & Operating Systems PBL
 * Author: moidrafath04 (Diagnostics & Evaluation Lead)
 */

#include "../include/page_replacement.h"

/**
 * @brief Locate the frame holding the page with the oldest reference timestamp
 */
static int find_lru_victim(const int last_used[], int frame_count) {
    int victim_frame = 0;
    int oldest_time = last_used[0];

    for (int f = 1; f < frame_count; f++) {
        if (last_used[f] < oldest_time) {
            oldest_time = last_used[f];
            victim_frame = f;
        }
    }

    return victim_frame;
}

/**
 * @brief Simulate Least Recently Used (LRU) Page Replacement
 * 
 * @param ref_str Reference string containing requested page numbers
 * @param frame_count Number of allocated physical frames
 * @return page_result_t Execution summary and step-by-step trace
 */
page_result_t simulate_lru(const ref_string_t *ref_str, int frame_count) {
    page_result_t res;
    memset(&res, 0, sizeof(res));
    res.algorithm = ALG_LRU;
    strncpy(res.name, "LRU", sizeof(res.name) - 1);
    res.total_references = ref_str->count;
    res.frame_count = frame_count;

    int frames[MAX_FRAMES];
    int last_used[MAX_FRAMES];
    init_frame_array(frames, frame_count);
    for (int f = 0; f < frame_count; f++) {
        last_used[f] = -1;
    }

    for (int i = 0; i < ref_str->count; i++) {
        int page = ref_str->pages[i];
        int logical_time = i; /* Monotonically increasing step timestamp */

        page_step_t *step = &res.steps[res.step_count++];
        step->step_index = i + 1;
        step->page = page;
        step->evicted_page = -1;

        int resident_idx = find_in_frames(frames, frame_count, page);

        if (resident_idx != -1) {
            /* Page Hit: Update reference time */
            res.hits++;
            step->is_hit = true;
            step->frame_index = resident_idx;
            last_used[resident_idx] = logical_time;
            snprintf(step->action, sizeof(step->action), "Page %d resident in Frame %d (refreshed)", 
                     page, resident_idx + 1);
        } else {
            /* Page Fault */
            res.faults++;
            step->is_hit = false;

            int empty_idx = find_empty_frame(frames, frame_count);
            if (empty_idx != -1) {
                /* Free frame available */
                frames[empty_idx] = page;
                last_used[empty_idx] = logical_time;
                step->frame_index = empty_idx;
                snprintf(step->action, sizeof(step->action), "Loaded page %d into Frame %d", 
                         page, empty_idx + 1);
            } else {
                /* Memory full: Evict frame with oldest access timestamp */
                int victim_frame = find_lru_victim(last_used, frame_count);
                int evicted = frames[victim_frame];
                frames[victim_frame] = page;
                last_used[victim_frame] = logical_time;

                step->evicted_page = evicted;
                step->frame_index = victim_frame;
                snprintf(step->action, sizeof(step->action), "Replaced page %d in Frame %d (LRU timestamp %d)", 
                         evicted, victim_frame + 1, victim_frame + 1);
            }
        }

        /* Snapshot current frame states */
        for (int f = 0; f < frame_count; f++) {
            step->frames[f] = frames[f];
        }
    }

    calculate_metrics(&res);
    return res;
}
