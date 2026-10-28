/**
 * @file page_cli.c
 * @brief Interactive CLI Menu & Console Driver for Page Replacement
 * 
 * Implements the student-facing terminal interface specified in the PBL problem
 * statement. Accepts page-reference strings, frame counts, algorithm choices,
 * and formats the step traces, performance metrics, and comparative evaluation.
 * 
 * Team 8 - Systems & Operating Systems PBL
 * Author: sneharao1811 (UI Frontend & Interactive Shell Lead)
 */

#include "../include/page_replacement.h"

#define MAX_LINE_BUF 1024

static void display_banner(void) {
    printf("===============================================================================\n");
    printf("     VIRTUAL MEMORY SIMULATOR: PAGE REPLACEMENT ALGORITHMS (PBL TEAM 8)         \n");
    printf("===============================================================================\n");
    printf("  Evaluates FIFO, LRU, and Optimal algorithms with per-reference frame traces, \n");
    printf("  hit/fault counters, hit and fault ratios, and comparative analysis.           \n");
    printf("===============================================================================\n\n");
}

int run_page_replacement_cli(int argc, char *argv[]) {
    display_banner();

    char ref_line[MAX_LINE_BUF];
    char frame_line[MAX_LINE_BUF];
    char choice_line[MAX_LINE_BUF];
    ref_string_t ref_str;
    int frame_count = 0;
    int choice = 0;

    /* Check for quick CLI demo mode: ./page_replacement_cli --demo */
    if (argc > 1 && strcmp(argv[1], "--demo") == 0) {
        printf("[DEMO MODE]: Running Silberschatz sample reference string with 3 frames...\n");
        const char *sample_ref = "7 0 1 2 0 3 0 4 2 3";
        parse_reference_tokens(sample_ref, &ref_str);
        frame_count = 3;
        choice = 4;
    } else if (argc > 1 && strcmp(argv[1], "--belady") == 0) {
        printf("[DEMO MODE]: Demonstrating Belady's Anomaly on FIFO (3 vs 4 frames)...\n");
        const char *belady_ref = "1 2 3 4 1 2 5 1 2 3 4 5";
        parse_reference_tokens(belady_ref, &ref_str);
        
        page_result_t f3 = simulate_fifo(&ref_str, 3);
        page_result_t f4 = simulate_fifo(&ref_str, 4);
        print_trace_table(&f3);
        print_performance_summary(&f3);
        print_trace_table(&f4);
        print_performance_summary(&f4);
        
        printf("\n===============================================================================\n");
        printf("  BELADY'S ANOMALY DEMONSTRATION SUMMARY:\n");
        printf("  Workload: 1 2 3 4 1 2 5 1 2 3 4 5\n");
        printf("  - Frames = 3: Faults = %d (Fault ratio: %.2f%%)\n", f3.faults, f3.fault_ratio);
        printf("  - Frames = 4: Faults = %d (Fault ratio: %.2f%%)\n", f4.faults, f4.fault_ratio);
        printf("  Notice: Increasing frames from 3 to 4 INCREASED page faults from 9 to 10!\n");
        printf("===============================================================================\n");
        return 0;
    } else {
        /* Standard Interactive Input */
        printf("Enter the page-reference string (space/comma separated):\n");
        if (!fgets(ref_line, sizeof(ref_line), stdin)) {
            fprintf(stderr, "Error reading page-reference string.\n");
            return 1;
        }

        int parsed = parse_reference_tokens(ref_line, &ref_str);
        if (parsed <= 0) {
            fprintf(stderr, "Error: Invalid reference string. Must contain at least one positive integer.\n");
            return 1;
        }

        printf("\nEnter the number of page frames (1 to %d):\n", MAX_FRAMES);
        if (!fgets(frame_line, sizeof(frame_line), stdin)) {
            fprintf(stderr, "Error reading frame count.\n");
            return 1;
        }

        frame_count = atoi(frame_line);
        if (frame_count < 1 || frame_count > MAX_FRAMES) {
            fprintf(stderr, "Error: Invalid frame count. Must be between 1 and %d.\n", MAX_FRAMES);
            return 1;
        }

        printf("\nSelect the algorithm:\n");
        printf("1. FIFO\n");
        printf("2. LRU\n");
        printf("3. Optimal\n");
        printf("4. Compare all algorithms\n");
        printf("\nChoice: ");

        if (!fgets(choice_line, sizeof(choice_line), stdin)) {
            fprintf(stderr, "Error reading algorithm choice.\n");
            return 1;
        }

        choice = atoi(choice_line);
        if (choice < 1 || choice > 4) {
            fprintf(stderr, "Error: Invalid selection. Choose 1, 2, 3, or 4.\n");
            return 1;
        }
    }

    /* Execution and Output Generation */
    if (choice == 1) {
        page_result_t fifo_res = simulate_fifo(&ref_str, frame_count);
        print_trace_table(&fifo_res);
        print_performance_summary(&fifo_res);
    } else if (choice == 2) {
        page_result_t lru_res = simulate_lru(&ref_str, frame_count);
        print_trace_table(&lru_res);
        print_performance_summary(&lru_res);
    } else if (choice == 3) {
        page_result_t opt_res = simulate_optimal(&ref_str, frame_count);
        print_trace_table(&opt_res);
        print_performance_summary(&opt_res);
    } else if (choice == 4) {
        page_result_t results[3];
        results[0] = simulate_fifo(&ref_str, frame_count);
        results[1] = simulate_lru(&ref_str, frame_count);
        results[2] = simulate_optimal(&ref_str, frame_count);

        for (int i = 0; i < 3; i++) {
            print_trace_table(&results[i]);
            print_performance_summary(&results[i]);
        }

        print_comparison_table(results, 3);
    }

    return 0;
}

int main(int argc, char *argv[]) {
    return run_page_replacement_cli(argc, argv);
}
