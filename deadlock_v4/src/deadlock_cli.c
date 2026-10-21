/**
 * @file deadlock_cli.c
 * @brief Interactive Terminal CLI for the Deadlock Simulation System.
 * 
 * Implements the 12-option interaction model specified in Week 4 curriculum.
 * Subsystem Lead: Sneha (UI & Frontend Lead)
 * OS Project-Based Learning (PBL) - Team 8
 */

#include "../include/deadlock.h"

static void clear_input_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

static void print_menu(void) {
    printf("========================================\n");
    printf("       DEADLOCK SIMULATION SYSTEM       \n");
    printf("========================================\n");
    printf(" 1. Create Processes\n");
    printf(" 2. Create Resources\n");
    printf(" 3. Enter Allocation Matrix\n");
    printf(" 4. Enter Maximum Matrix\n");
    printf(" 5. Display Need Matrix\n");
    printf(" 6. Display Resource Allocation Graph\n");
    printf(" 7. Request Resources\n");
    printf(" 8. Run Banker's Algorithm\n");
    printf(" 9. Detect Deadlock\n");
    printf("10. Recover from Deadlock\n");
    printf("11. Display System Status\n");
    printf("12. Exit\n");
    printf("----------------------------------------\n");
    printf("Quick Presets: [T]extbook Safe | [D]eadlock Cycle | [U]nsafe State\n");
    printf("Enter choice: ");
}

int main(void) {
    deadlock_sys_t sys;
    deadlock_sys_init(&sys);

    /* Start with standard textbook preset so the system is immediately usable */
    deadlock_sys_load_textbook_preset(&sys);

    printf("\n[System Initialized with Standard 5-Process, 3-Resource Benchmark Scenario]\n\n");

    char line[64];
    bool running = true;

    while (running) {
        print_menu();
        if (!fgets(line, sizeof(line), stdin)) {
            break;
        }

        /* Check quick presets */
        if (line[0] == 'T' || line[0] == 't') {
            deadlock_sys_load_textbook_preset(&sys);
            printf("\n[Loaded Standard Textbook Safe Scenario (5 processes, 3 resources)]\n\n");
            continue;
        } else if (line[0] == 'D' || line[0] == 'd') {
            deadlock_sys_load_deadlock_preset(&sys);
            printf("\n[Loaded Deadlocked Scenario (3 processes in circular wait)]\n\n");
            continue;
        } else if (line[0] == 'U' || line[0] == 'u') {
            deadlock_sys_load_unsafe_preset(&sys);
            printf("\n[Loaded Unsafe Scenario]\n\n");
            continue;
        }

        int choice = atoi(line);

        switch (choice) {
            case 1: { /* Create Processes */
                int np = 0;
                printf("Enter number of processes (1 - %d): ", MAX_PROCESSES);
                if (scanf("%d", &np) == 1 && np > 0 && np <= MAX_PROCESSES) {
                    sys.num_processes = np;
                    for (int i = 0; i < np; i++) {
                        snprintf(sys.process_names[i], MAX_NAME_LEN, "P%d", i);
                        sys.process_active[i] = true;
                    }
                    deadlock_sys_reset_matrices(&sys);
                    printf("Successfully created %d processes.\n\n", np);
                } else {
                    printf("Invalid number of processes.\n\n");
                }
                clear_input_buffer();
                break;
            }

            case 2: { /* Create Resources */
                int nr = 0;
                printf("Enter number of resource types (1 - %d): ", MAX_RESOURCES);
                if (scanf("%d", &nr) == 1 && nr > 0 && nr <= MAX_RESOURCES) {
                    sys.num_resources = nr;
                    for (int j = 0; j < nr; j++) {
                        snprintf(sys.resource_names[j], MAX_NAME_LEN, "R%d", j);
                        printf("Enter total capacity for resource %s: ", sys.resource_names[j]);
                        if (scanf("%d", &sys.total_resources[j]) != 1 || sys.total_resources[j] < 0) {
                            sys.total_resources[j] = 0;
                        }
                        sys.available[j] = sys.total_resources[j];
                    }
                    printf("Successfully created %d resource types.\n\n", nr);
                } else {
                    printf("Invalid number of resources.\n\n");
                }
                clear_input_buffer();
                break;
            }

            case 3: { /* Enter Allocation Matrix */
                if (sys.num_processes == 0 || sys.num_resources == 0) {
                    printf("Please create processes and resources first.\n\n");
                    break;
                }
                printf("Enter Allocation Matrix (%d x %d):\n", sys.num_processes, sys.num_resources);
                for (int i = 0; i < sys.num_processes; i++) {
                    printf("  Process %s (%d values): ", sys.process_names[i], sys.num_resources);
                    for (int j = 0; j < sys.num_resources; j++) {
                        if (scanf("%d", &sys.allocation[i][j]) != 1) {
                            sys.allocation[i][j] = 0;
                        }
                    }
                }
                clear_input_buffer();
                deadlock_sys_recalculate_need(&sys);
                deadlock_sys_recalculate_available(&sys);
                printf("Allocation matrix updated successfully.\n\n");
                break;
            }

            case 4: { /* Enter Maximum Matrix */
                if (sys.num_processes == 0 || sys.num_resources == 0) {
                    printf("Please create processes and resources first.\n\n");
                    break;
                }
                printf("Enter Maximum Claim Matrix (%d x %d):\n", sys.num_processes, sys.num_resources);
                for (int i = 0; i < sys.num_processes; i++) {
                    printf("  Process %s (%d values): ", sys.process_names[i], sys.num_resources);
                    for (int j = 0; j < sys.num_resources; j++) {
                        if (scanf("%d", &sys.maximum[i][j]) != 1) {
                            sys.maximum[i][j] = 0;
                        }
                    }
                }
                clear_input_buffer();
                deadlock_sys_recalculate_need(&sys);
                printf("Maximum matrix updated and Need matrix recalculated.\n\n");
                break;
            }

            case 5: { /* Display Need Matrix */
                deadlock_sys_recalculate_need(&sys);
                deadlock_sys_recalculate_available(&sys);
                deadlock_sys_print_matrices(&sys);
                break;
            }

            case 6: { /* Display Resource Allocation Graph */
                {
                    rag_graph_t graph;
                    rag_build_graph(&sys, &graph);
                    rag_print_ascii(&sys, &graph);
                }
                break;
            }

            case 7: { /* Request Resources */
                if (sys.num_processes == 0 || sys.num_resources == 0) {
                    printf("System has no processes or resources.\n\n");
                    break;
                }
                int pid = 0;
                printf("Enter requesting Process index (0 to %d): ", sys.num_processes - 1);
                if (scanf("%d", &pid) != 1 || pid < 0 || pid >= sys.num_processes || !sys.process_active[pid]) {
                    printf("Invalid process index.\n\n");
                    clear_input_buffer();
                    break;
                }
                int req[MAX_RESOURCES];
                printf("Enter request vector (%d resource instances): ", sys.num_resources);
                for (int j = 0; j < sys.num_resources; j++) {
                    if (scanf("%d", &req[j]) != 1) req[j] = 0;
                }
                clear_input_buffer();

                char result_msg[512];
                bankers_request_resource(&sys, pid, req, result_msg, sizeof(result_msg));
                printf("\n>>> %s\n\n", result_msg);
                break;
            }

            case 8: { /* Run Banker's Algorithm */
                safety_result_t safe = bankers_check_safety(&sys);
                printf("\n========================================================================\n");
                printf("                   BANKER'S SAFETY ALGORITHM EVALUATION                \n");
                printf("========================================================================\n");
                if (safe.is_safe) {
                    printf("Result        : \033[1;32mSYSTEM IS IN A SAFE STATE\033[0m\n");
                    printf("Safe Sequence : < ");
                    for (int k = 0; k < safe.sequence_len; k++) {
                        printf("%s%s", sys.process_names[safe.safe_sequence[k]],
                               (k == safe.sequence_len - 1) ? "" : ", ");
                    }
                    printf(" >\n");
                } else {
                    printf("Result        : \033[1;33mSYSTEM IS IN AN UNSAFE STATE\033[0m\n");
                }
                printf("Analysis      : %s\n", safe.explanation);
                printf("========================================================================\n\n");
                break;
            }

            case 9: { /* Detect Deadlock */
                detection_result_t det = deadlock_detect(&sys);
                printf("\n========================================================================\n");
                printf("                     DEADLOCK DETECTION ALGORITHM                      \n");
                printf("========================================================================\n");
                if (det.deadlock_detected) {
                    printf("Verdict       : \033[1;31mDEADLOCK DETECTED!\033[0m\n");
                    printf("Deadlocked PIDs (%d): ", det.deadlocked_count);
                    for (int k = 0; k < det.deadlocked_count; k++) {
                        printf("%s ", sys.process_names[det.deadlocked_processes[k]]);
                    }
                    printf("\n");
                } else {
                    printf("Verdict       : \033[1;32mNO DEADLOCK DETECTED\033[0m\n");
                }
                printf("Diagnosis     : %s\n", det.explanation);
                printf("========================================================================\n\n");
                break;
            }

            case 10: { /* Recover from Deadlock */
                printf("\nSelect Deadlock Recovery Strategy:\n");
                printf("  1. Terminate All Deadlocked Processes\n");
                printf("  2. Selectively Abort Victim One-by-One (Lowest Cost / Max Reclaimed)\n");
                printf("  3. Preempt Resources from Victim and Rollback\n");
                printf("Enter choice (1-3): ");
                int strat_choice = 0;
                if (scanf("%d", &strat_choice) == 1 && strat_choice >= 1 && strat_choice <= 3) {
                    char recovery_log[512];
                    deadlock_recover(&sys, (recovery_strategy_t)strat_choice, recovery_log, sizeof(recovery_log));
                    printf("\n>>> %s\n\n", recovery_log);
                } else {
                    printf("Invalid strategy selected.\n\n");
                }
                clear_input_buffer();
                break;
            }

            case 11: { /* Display System Status */
                deadlock_sys_print_status(&sys);
                break;
            }

            case 12: { /* Exit */
                printf("\nExiting Deadlock Simulation System. Goodbye!\n");
                running = false;
                break;
            }

            default:
                printf("Invalid selection (%d). Please enter a choice between 1 and 12.\n\n", choice);
                break;
        }
    }

    return 0;
}
