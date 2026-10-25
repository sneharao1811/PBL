/**
 * @file page_replacement.h
 * @brief Page Replacement Simulation Architecture Header
 * 
 * Defines core state models, trace data structures, algorithm identifiers,
 * and function prototypes for FIFO, LRU, and Optimal page replacement algorithms.
 * 
 * Team 8 - Systems & Operating Systems PBL
 * Lead Architect: Waqqas (System Integration & Architecture)
 */

#ifndef PAGE_REPLACEMENT_H
#define PAGE_REPLACEMENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_REFERENCES 512
#define MAX_FRAMES     64
#define EMPTY_FRAME    -1

/**
 * @brief Supported Page Replacement Algorithms
 */
typedef enum {
    ALG_FIFO,
    ALG_LRU,
    ALG_OPTIMAL
} page_alg_t;

/**
 * @brief Trace snapshot at a single reference step
 */
typedef struct {
    int step_index;              /**< 1-based step counter */
    int page;                    /**< Requested page number */
    int frames[MAX_FRAMES];      /**< Snapshot of frames after reference */
    bool is_hit;                 /**< True if page was already resident */
    int evicted_page;            /**< Evicted page number, or -1 if none */
    int frame_index;             /**< Frame index where page was loaded/hit */
    char action[96];             /**< Human-readable action description */
} page_step_t;

/**
 * @brief Comprehensive simulation results for an algorithm
 */
typedef struct {
    page_alg_t algorithm;        /**< Algorithm type */
    char name[32];               /**< Display name */
    int total_references;        /**< Total page requests processed */
    int frame_count;             /**< Number of physical memory frames */
    int hits;                    /**< Total page hits */
    int faults;                  /**< Total page faults */
    double hit_ratio;            /**< hits / total * 100.0 */
    double fault_ratio;          /**< faults / total * 100.0 */
    int step_count;              /**< Number of trace steps recorded */
    page_step_t steps[MAX_REFERENCES]; /**< Chronological step trace */
} page_result_t;

/**
 * @brief Parsed reference string container
 */
typedef struct {
    int pages[MAX_REFERENCES];
    int count;
} ref_string_t;

/* Core Engine & Utilities (page_core.c) */
int parse_reference_tokens(const char *input, ref_string_t *ref_str);
void init_frame_array(int frames[], int count);
int find_in_frames(const int frames[], int count, int page);
int find_empty_frame(const int frames[], int count);
void calculate_metrics(page_result_t *res);
void print_trace_table(const page_result_t *res);
void print_performance_summary(const page_result_t *res);
void print_comparison_table(const page_result_t results[], int count);

/* Algorithmic Implementations */
page_result_t simulate_fifo(const ref_string_t *ref_str, int frame_count);
page_result_t simulate_lru(const ref_string_t *ref_str, int frame_count);
page_result_t simulate_optimal(const ref_string_t *ref_str, int frame_count);

/* Interactive Shell (page_cli.c) */
int run_page_replacement_cli(int argc, char *argv[]);

#endif /* PAGE_REPLACEMENT_H */
