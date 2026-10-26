/**
 * @file fifo.c
 * @brief First-In First-Out (FIFO) Page Replacement Algorithm
 * 
 * Implements standard FIFO replacement policy using a circular replacement pointer.
 * Tracks queue insertion order without modifying queue position on page hits.
 * Subject to Belady's Anomaly under specific non-stack access patterns.
 * 
 * Team 8 - Systems & Operating Systems PBL
 * Author: mallyasiddanth (Core Algorithms Lead)
 */

#include "../include/page_replacement.h"

/**
 * @brief Simulate First-In First-Out (FIFO) Page Replacement
 * 
 * @param ref_str Reference string containing requested page numbers
 * @param frame_count Number of allocated physical frames
 * @return page_result_t Execution summary and step-by-step trace
 */
page_result_t simulate_fifo(const ref_string_t *ref_str, int frame_count) {
    page_result_t res;
    memset(&res, 0, sizeof(res));
    res.algorithm = ALG_FIFO;
    strncpy(res.name, "FIFO", sizeof(res.name) - 1);
    res.total_references = ref_str->count;
    res.frame_count = frame_count;

    int frames[MAX_FRAMES];
    init_frame_array(frames, frame_count);

    int fifo_pointer = 0;  /* Points to the oldest allocated frame */

    for (int i = 0; i < ref_str->count; i++) {
        int page = ref_str->pages[i];
        page_step_t *step = &res.steps[res.step_count++];
        step->step_index = i + 1;
        step->page = page;
        step->evicted_page = -1;

        int resident_idx = find_in_frames(frames, frame_count, page);

        if (resident_idx != -1) {
            /* Page Hit: Page is already resident in frames */
            res.hits++;
            step->is_hit = true;
            step->frame_index = resident_idx;
            snprintf(step->action, sizeof(step->action), "Page %d resident in Frame %d", 
                     page, resident_idx + 1);
        } else {
            /* Page Fault: Page absent from memory */
            res.faults++;
            step->is_hit = false;

            int empty_idx = find_empty_frame(frames, frame_count);
            if (empty_idx != -1) {
                /* Free frame available: allocate directly without replacement */
                frames[empty_idx] = page;
                step->frame_index = empty_idx;
                snprintf(step->action, sizeof(step->action), "Loaded page %d into Frame %d", 
                         page, empty_idx + 1);
            } else {
                /* Memory full: replace victim at fifo_pointer */
                int victim_frame = fifo_pointer;
                int evicted = frames[victim_frame];
                frames[victim_frame] = page;

                step->evicted_page = evicted;
                step->frame_index = victim_frame;
                snprintf(step->action, sizeof(step->action), "Replaced page %d in Frame %d", 
                         evicted, victim_frame + 1);

                /* Advance circular FIFO queue pointer */
                fifo_pointer = (fifo_pointer + 1) % frame_count;
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
