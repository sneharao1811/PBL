/**
 * @file page_core.c
 * @brief Core Simulation Engine, Reference Parser, and Output Formatters
 * 
 * Provides reference string parsing, frame array management, hit/fault metric
 * calculations, and table formatters matching the curriculum specification.
 * 
 * Team 8 - Systems & Operating Systems PBL
 * Author: Waqqas H (Lead Systems Architect)
 */

#include "../include/page_replacement.h"
#include <ctype.h>

/**
 * @brief Parse whitespace or comma-separated tokens into page reference numbers
 */
int parse_reference_tokens(const char *input, ref_string_t *ref_str) {
    if (!input || !ref_str) return -1;
    ref_str->count = 0;

    const char *ptr = input;
    while (*ptr != '\0') {
        while (*ptr && (isspace((unsigned char)*ptr) || *ptr == ',')) {
            ptr++;
        }
        if (*ptr == '\0') break;

        if (!isdigit((unsigned char)*ptr)) {
            return -1; /* Non-numeric token */
        }

        long val = strtol(ptr, (char **)&ptr, 10);
        if (val < 0) return -1;

        if (ref_str->count >= MAX_REFERENCES) {
            return -2; /* Capacity exceeded */
        }
        ref_str->pages[ref_str->count++] = (int)val;
    }

    return ref_str->count;
}

/**
 * @brief Initialize all frames to EMPTY_FRAME (-1)
 */
void init_frame_array(int frames[], int count) {
    for (int i = 0; i < count; i++) {
        frames[i] = EMPTY_FRAME;
    }
}

/**
 * @brief Search if a page is currently resident in memory frames
 * @return Frame index if present, or -1 if absent
 */
int find_in_frames(const int frames[], int count, int page) {
    for (int i = 0; i < count; i++) {
        if (frames[i] == page) return i;
    }
    return -1;
}

/**
 * @brief Find the first available empty frame slot
 * @return Lowest empty frame index, or -1 if full
 */
int find_empty_frame(const int frames[], int count) {
    for (int i = 0; i < count; i++) {
        if (frames[i] == EMPTY_FRAME) return i;
    }
    return -1;
}

/**
 * @brief Compute hit ratio and fault ratio percentages
 */
void calculate_metrics(page_result_t *res) {
    if (res->total_references > 0) {
        res->hit_ratio = ((double)res->hits / (double)res->total_references) * 100.0;
        res->fault_ratio = ((double)res->faults / (double)res->total_references) * 100.0;
    } else {
        res->hit_ratio = 0.0;
        res->fault_ratio = 0.0;
    }
}

/**
 * @brief Print chronological step-by-step trace table
 */
void print_trace_table(const page_result_t *res) {
    if (!res || res->step_count == 0) return;

    printf("\n===============================================================================\n");
    printf("  SIMULATION TRACE: %s (%d Frames, %d References)\n", 
           res->name, res->frame_count, res->total_references);
    printf("===============================================================================\n");

    /* Header */
    printf("| %-4s | %-5s |", "Step", "Page");
    for (int f = 0; f < res->frame_count; f++) {
        printf(" Frame %-2d |", f + 1);
    }
    printf(" %-7s | %-32s |\n", "Status", "Action");

    /* Divider */
    printf("|------|-------|");
    for (int f = 0; f < res->frame_count; f++) {
        printf("----------|");
    }
    printf("---------|----------------------------------|\n");

    /* Rows */
    for (int s = 0; s < res->step_count; s++) {
        const page_step_t *step = &res->steps[s];
        printf("| %-4d | %-5d |", step->step_index, step->page);
        for (int f = 0; f < res->frame_count; f++) {
            if (step->frames[f] == EMPTY_FRAME) {
                printf(" %-8s |", "-");
            } else {
                printf(" %-8d |", step->frames[f]);
            }
        }
        printf(" %-7s | %-32s |\n", 
               step->is_hit ? "Hit" : "Fault", 
               step->action);
    }
    printf("-------------------------------------------------------------------------------\n");
}

/**
 * @brief Print performance summary table matching curriculum spec
 */
void print_performance_summary(const page_result_t *res) {
    if (!res) return;

    printf("\n-------------------------------------------------------------------------------\n");
    printf("  PERFORMANCE SUMMARY (%s)\n", res->name);
    printf("-------------------------------------------------------------------------------\n");
    printf("| %-32s | %-15s |\n", "Metric", "Expected output");
    printf("|----------------------------------|-----------------|\n");
    printf("| %-32s | %-15d |\n", "Total page references", res->total_references);
    printf("| Total page hits (%-14s) | %-15d |\n", res->name, res->hits);
    printf("| Total page faults (%-12s) | %-15d |\n", res->name, res->faults);
    printf("| Page hit ratio (%-14s) | %6.2f%%         |\n", res->name, res->hit_ratio);
    printf("| Page fault ratio (%-12s) | %6.2f%%         |\n", res->name, res->fault_ratio);
    printf("-------------------------------------------------------------------------------\n");
}

/**
 * @brief Print comparative evaluation table across multiple algorithms
 */
void print_comparison_table(const page_result_t results[], int count) {
    if (!results || count <= 0) return;

    int min_faults = results[0].faults;
    for (int i = 1; i < count; i++) {
        if (results[i].faults < min_faults) {
            min_faults = results[i].faults;
        }
    }

    printf("\n===============================================================================\n");
    printf("  ALGORITHM COMPARATIVE EVALUATION SUMMARY\n");
    printf("===============================================================================\n");
    printf("| %-12s | %-10s | %-12s | %-12s | %-12s |\n", 
           "Algorithm", "Page Hits", "Page Faults", "Hit Ratio", "Fault Ratio");
    printf("|--------------|------------|--------------|--------------|--------------|\n");

    for (int i = 0; i < count; i++) {
        const page_result_t *r = &results[i];
        printf("| %-12s | %-10d | %-12d | %9.2f%%   | %9.2f%%   |%s\n",
               r->name, r->hits, r->faults, r->hit_ratio, r->fault_ratio,
               (r->faults == min_faults) ? " <= BEST" : "");
    }
    printf("===============================================================================\n");

    printf("\n[Analysis Note]:\n");
    printf(" - Optimal replacement establishes the theoretical lower bound on page faults.\n");
    printf(" - LRU leverages temporal locality, approximating Optimal for realistic workloads.\n");
    printf(" - FIFO suffers from Belady's Anomaly where increasing frames may increase faults.\n");
}
