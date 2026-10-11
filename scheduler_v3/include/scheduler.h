#ifndef SCHEDULER_H
#define SCHEDULER_H

#define MAX_PROCESSES 64
#define MAX_GANTT_SLICES 512

typedef struct {
    int pid;
    int arrival_time;
    int burst_time;
    int remaining_time;
    int priority;           // Lower integer = Higher priority (standard UNIX/POSIX convention)
    int start_time;
    int completion_time;
    int waiting_time;
    int turnaround_time;
    int response_time;
    int is_completed;
} process_t;

typedef struct {
    int pid;        // -1 indicates CPU idle
    int start_time;
    int end_time;
} gantt_slice_t;

typedef struct {
    process_t procs[MAX_PROCESSES];
    int count;
    int time_quantum; // For Round Robin
    gantt_slice_t gantt[MAX_GANTT_SLICES];
    int gantt_count;
    double avg_waiting_time;
    double avg_turnaround_time;
    double avg_response_time;
    double cpu_utilization;
} schedule_result_t;

// Scheduling Algorithm Prototypes
schedule_result_t run_fcfs(const process_t procs[], int n);
schedule_result_t run_sjf_non_preemptive(const process_t procs[], int n);
schedule_result_t run_sjf_preemptive(const process_t procs[], int n);
schedule_result_t run_priority_non_preemptive(const process_t procs[], int n);
schedule_result_t run_priority_preemptive(const process_t procs[], int n);
schedule_result_t run_round_robin(const process_t procs[], int n, int quantum);

// Utilities & Display
void display_results(const char *algo_name, const schedule_result_t *res);
void print_gantt_chart(const schedule_result_t *res);

#endif // SCHEDULER_H
