/**
 * @file optimal.c
 * @brief Belady's Optimal Page Replacement Algorithm (MIN)
 * 
 * Implements Belady's clairvoyant optimal replacement algorithm by inspecting
 * the future reference string to identify the page whose next reference occurs
 * farthest in the future (or is never referenced again).
 * Represents the theoretical lower bound on page faults for any offline reference string.
 * 
 * Team 8 - Systems & Operating Systems PBL
 * Author: mallyasiddanth (Core Algorithms Lead)
 */

#include "../include/page_replacement.h"

/**
 * @brief Select the victim frame whose page will not be used for the longest period of time
 */
static int find_optimal_victim(const int frames[], int frame_count, 
                               const ref_string_t *ref_str, int current_index) {
    int victim_frame = 0;
    int farthest_next_use = -1;

    for (int f = 0; f < frame_count; f++) {
        int resident_page = frames[f];
        int next_use = -1;

        /* Scan future references for the first occurrence of this resident page */
        for (int future = current_index + 1; future < ref_str->count; future++) {
            if (ref_str->pages[future] == resident_page) {
                next_use = future;
                break;
            }
        }

        /* If page is NEVER used again in the future, it is the best possible victim */
        if (next_use == -1) {
            return f;
        }

        /* Otherwise, track page with the farthest future reference */
        if (next_use > farthest_next_use) {
            farthest_next_use = next_use;
            victim_frame = f;
        }
    }

    return victim_frame;
}

/**
 * @brief Simulate Belady's Optimal (OPT) Page Replacement
 * 
 * @param ref_str Reference string containing requested page numbers
 * @param frame_count Number of allocated physical frames
 * @return page_result_t Execution summary and step-by-step trace
 */
page_result_t simulate_optimal(const ref_string_t *ref_str, int frame_count) {
    page_result_t res;
    memset(&res, 0, sizeof(res));
    res.algorithm = ALG_OPTIMAL;
    strncpy(res.name, "Optimal", sizeof(res.name) - 1);
    res.total_references = ref_str->count;
    res.frame_count = frame_count;

    int frames[MAX_FRAMES];
    init_frame_array(frames, frame_count);

    for (int i = 0; i < ref_str->count; i++) {
        int page = ref_str->pages[i];
        page_step_t *step = &res.steps[res.step_count++];
        step->step_index = i + 1;
        step->page = page;
        step->evicted_page = -1;

        int resident_idx = find_in_frames(frames, frame_count, page);

        if (resident_idx != -1) {
            /* Page Hit: Page already resident */
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
                /* Free frame available */
                frames[empty_idx] = page;
                step->frame_index = empty_idx;
                snprintf(step->action, sizeof(step->action), "Loaded page %d into Frame %d", 
                         page, empty_idx + 1);
            } else {
                /* Memory full: Clairvoyant lookahead victim selection */
                int victim_frame = find_optimal_victim(frames, frame_count, ref_str, i);
                int evicted = frames[victim_frame];
                frames[victim_frame] = page;

                step->evicted_page = evicted;
                step->frame_index = victim_frame;
                snprintf(step->action, sizeof(step->action), "Replaced page %d in Frame %d (MIN lookahead)", 
                         evicted, victim_frame + 1);
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
