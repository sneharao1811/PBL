#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../include/scheduler.h"

static void add_gantt_slice(schedule_result_t *res, int pid, int start, int end) {
    if (start >= end) return;
    if (res->gantt_count > 0 && res->gantt[res->gantt_count - 1].pid == pid && res->gantt[res->gantt_count - 1].end_time == start) {
        res->gantt[res->gantt_count - 1].end_time = end;
    } else {
        if (res->gantt_count < MAX_GANTT_SLICES) {
            res->gantt[res->gantt_count].pid = pid;
            res->gantt[res->gantt_count].start_time = start;
            res->gantt[res->gantt_count].end_time = end;
            res->gantt_count++;
        }
    }
}

static void finalize_metrics(schedule_result_t *res) {
    double total_wt = 0, total_tat = 0, total_rt = 0;
    int busy_time = 0;
    int total_time = 0;

    for (int i = 0; i < res->count; i++) {
        total_wt  += res->procs[i].waiting_time;
        total_tat += res->procs[i].turnaround_time;
        total_rt  += res->procs[i].response_time;
        busy_time += res->procs[i].burst_time;
    }

    if (res->gantt_count > 0) {
        total_time = res->gantt[res->gantt_count - 1].end_time;
    }

    res->avg_waiting_time = (res->count > 0) ? (total_wt / res->count) : 0;
    res->avg_turnaround_time = (res->count > 0) ? (total_tat / res->count) : 0;
    res->avg_response_time = (res->count > 0) ? (total_rt / res->count) : 0;
    res->cpu_utilization = (total_time > 0) ? ((double)busy_time / total_time * 100.0) : 0;
}

// 1. FCFS
schedule_result_t run_fcfs(const process_t procs[], int n) {
    schedule_result_t res;
    memset(&res, 0, sizeof(res));
    res.count = n;
    for (int i = 0; i < n; i++) {
        res.procs[i] = procs[i];
        res.procs[i].remaining_time = procs[i].burst_time;
        res.procs[i].is_completed = 0;
    }

    int current_time = 0;
    int completed = 0;

    while (completed < n) {
        int best = -1;
        int min_arrival = 1e9;
        for (int i = 0; i < n; i++) {
            if (!res.procs[i].is_completed && res.procs[i].arrival_time <= current_time) {
                if (res.procs[i].arrival_time < min_arrival) {
                    min_arrival = res.procs[i].arrival_time;
                    best = i;
                }
            }
        }

        if (best == -1) {
            // Find next arrival time
            int next_arr = 1e9;
            for (int i = 0; i < n; i++) {
                if (!res.procs[i].is_completed && res.procs[i].arrival_time < next_arr) {
                    next_arr = res.procs[i].arrival_time;
                }
            }
            add_gantt_slice(&res, -1, current_time, next_arr);
            current_time = next_arr;
            continue;
        }

        res.procs[best].start_time = current_time;
        res.procs[best].response_time = res.procs[best].start_time - res.procs[best].arrival_time;
        int end = current_time + res.procs[best].burst_time;
        add_gantt_slice(&res, res.procs[best].pid, current_time, end);

        current_time = end;
        res.procs[best].completion_time = current_time;
        res.procs[best].turnaround_time = res.procs[best].completion_time - res.procs[best].arrival_time;
        res.procs[best].waiting_time = res.procs[best].turnaround_time - res.procs[best].burst_time;
        res.procs[best].is_completed = 1;
        completed++;
    }

    finalize_metrics(&res);
    return res;
}


// 2. SJF Non-Preemptive
schedule_result_t run_sjf_non_preemptive(const process_t procs[], int n) {
    schedule_result_t res;
    memset(&res, 0, sizeof(res));
    res.count = n;
    for (int i = 0; i < n; i++) {
        res.procs[i] = procs[i];
        res.procs[i].is_completed = 0;
    }

    int current_time = 0, completed = 0;
    while (completed < n) {
        int best = -1;
        int min_burst = 1e9;
        for (int i = 0; i < n; i++) {
            if (!res.procs[i].is_completed && res.procs[i].arrival_time <= current_time) {
                if (res.procs[i].burst_time < min_burst) {
                    min_burst = res.procs[i].burst_time;
                    best = i;
                }
            }
        }

        if (best == -1) {
            int next_arr = 1e9;
            for (int i = 0; i < n; i++) {
                if (!res.procs[i].is_completed && res.procs[i].arrival_time < next_arr) {
                    next_arr = res.procs[i].arrival_time;
                }
            }
            add_gantt_slice(&res, -1, current_time, next_arr);
            current_time = next_arr;
            continue;
        }

        res.procs[best].start_time = current_time;
        res.procs[best].response_time = res.procs[best].start_time - res.procs[best].arrival_time;
        int end = current_time + res.procs[best].burst_time;
        add_gantt_slice(&res, res.procs[best].pid, current_time, end);

        current_time = end;
        res.procs[best].completion_time = current_time;
        res.procs[best].turnaround_time = res.procs[best].completion_time - res.procs[best].arrival_time;
        res.procs[best].waiting_time = res.procs[best].turnaround_time - res.procs[best].burst_time;
        res.procs[best].is_completed = 1;
        completed++;
    }

    finalize_metrics(&res);
    return res;
}

// 3. SJF Preemptive (SRTF)
schedule_result_t run_sjf_preemptive(const process_t procs[], int n) {
    schedule_result_t res;
    memset(&res, 0, sizeof(res));
    res.count = n;
    for (int i = 0; i < n; i++) {
        res.procs[i] = procs[i];
        res.procs[i].remaining_time = procs[i].burst_time;
        res.procs[i].start_time = -1;
        res.procs[i].is_completed = 0;
    }

    int current_time = 0, completed = 0;
    int last_pid = -2;
    int slice_start = 0;

    while (completed < n) {
        int best = -1;
        int min_rem = 1e9;
        for (int i = 0; i < n; i++) {
            if (!res.procs[i].is_completed && res.procs[i].arrival_time <= current_time) {
                if (res.procs[i].remaining_time < min_rem) {
                    min_rem = res.procs[i].remaining_time;
                    best = i;
                }
            }
        }

        int curr_pid = (best != -1) ? res.procs[best].pid : -1;
        if (curr_pid != last_pid) {
            if (last_pid != -2) {
                add_gantt_slice(&res, last_pid, slice_start, current_time);
            }
            slice_start = current_time;
            last_pid = curr_pid;
        }

        if (best == -1) {
            current_time++;
            continue;
        }

        if (res.procs[best].start_time == -1) {
            res.procs[best].start_time = current_time;
            res.procs[best].response_time = current_time - res.procs[best].arrival_time;
        }

        res.procs[best].remaining_time--;
        current_time++;

        if (res.procs[best].remaining_time == 0) {
            res.procs[best].completion_time = current_time;
            res.procs[best].turnaround_time = res.procs[best].completion_time - res.procs[best].arrival_time;
            res.procs[best].waiting_time = res.procs[best].turnaround_time - res.procs[best].burst_time;
            res.procs[best].is_completed = 1;
            completed++;
        }
    }

    if (last_pid != -2) {
        add_gantt_slice(&res, last_pid, slice_start, current_time);
    }

    finalize_metrics(&res);
    return res;
}


// 4. Priority Non-Preemptive
schedule_result_t run_priority_non_preemptive(const process_t procs[], int n) {
    schedule_result_t res;
    memset(&res, 0, sizeof(res));
    res.count = n;
    for (int i = 0; i < n; i++) {
        res.procs[i] = procs[i];
        res.procs[i].is_completed = 0;
    }

    int current_time = 0, completed = 0;
    while (completed < n) {
        int best = -1;
        int best_prio = 1e9;
        for (int i = 0; i < n; i++) {
            if (!res.procs[i].is_completed && res.procs[i].arrival_time <= current_time) {
                if (res.procs[i].priority < best_prio) {
                    best_prio = res.procs[i].priority;
                    best = i;
                }
            }
        }

        if (best == -1) {
            int next_arr = 1e9;
            for (int i = 0; i < n; i++) {
                if (!res.procs[i].is_completed && res.procs[i].arrival_time < next_arr) {
                    next_arr = res.procs[i].arrival_time;
                }
            }
            add_gantt_slice(&res, -1, current_time, next_arr);
            current_time = next_arr;
            continue;
        }

        res.procs[best].start_time = current_time;
        res.procs[best].response_time = res.procs[best].start_time - res.procs[best].arrival_time;
        int end = current_time + res.procs[best].burst_time;
        add_gantt_slice(&res, res.procs[best].pid, current_time, end);

        current_time = end;
        res.procs[best].completion_time = current_time;
        res.procs[best].turnaround_time = res.procs[best].completion_time - res.procs[best].arrival_time;
        res.procs[best].waiting_time = res.procs[best].turnaround_time - res.procs[best].burst_time;
        res.procs[best].is_completed = 1;
        completed++;
    }

    finalize_metrics(&res);
    return res;
}

// 5. Priority Preemptive
schedule_result_t run_priority_preemptive(const process_t procs[], int n) {
    schedule_result_t res;
    memset(&res, 0, sizeof(res));
    res.count = n;
    for (int i = 0; i < n; i++) {
        res.procs[i] = procs[i];
        res.procs[i].remaining_time = procs[i].burst_time;
        res.procs[i].start_time = -1;
        res.procs[i].is_completed = 0;
    }

    int current_time = 0, completed = 0;
    int last_pid = -2;
    int slice_start = 0;

    while (completed < n) {
        int best = -1;
        int best_prio = 1e9;
        for (int i = 0; i < n; i++) {
            if (!res.procs[i].is_completed && res.procs[i].arrival_time <= current_time) {
                if (res.procs[i].priority < best_prio) {
                    best_prio = res.procs[i].priority;
                    best = i;
                }
            }
        }

        int curr_pid = (best != -1) ? res.procs[best].pid : -1;
        if (curr_pid != last_pid) {
            if (last_pid != -2) {
                add_gantt_slice(&res, last_pid, slice_start, current_time);
            }
            slice_start = current_time;
            last_pid = curr_pid;
        }

        if (best == -1) {
            current_time++;
            continue;
        }

        if (res.procs[best].start_time == -1) {
            res.procs[best].start_time = current_time;
            res.procs[best].response_time = current_time - res.procs[best].arrival_time;
        }

        res.procs[best].remaining_time--;
        current_time++;

        if (res.procs[best].remaining_time == 0) {
            res.procs[best].completion_time = current_time;
            res.procs[best].turnaround_time = res.procs[best].completion_time - res.procs[best].arrival_time;
            res.procs[best].waiting_time = res.procs[best].turnaround_time - res.procs[best].burst_time;
            res.procs[best].is_completed = 1;
            completed++;
        }
    }

    if (last_pid != -2) {
        add_gantt_slice(&res, last_pid, slice_start, current_time);
    }

    finalize_metrics(&res);
    return res;
}


// 6. Round Robin
schedule_result_t run_round_robin(const process_t procs[], int n, int quantum) {
    schedule_result_t res;
    memset(&res, 0, sizeof(res));
    res.count = n;
    res.time_quantum = quantum;

    for (int i = 0; i < n; i++) {
        res.procs[i] = procs[i];
        res.procs[i].remaining_time = procs[i].burst_time;
        res.procs[i].start_time = -1;
        res.procs[i].is_completed = 0;
    }

    int queue[MAX_PROCESSES * 8];
    int q_head = 0, q_tail = 0;
    bool in_queue[MAX_PROCESSES] = {false};

    int current_time = 0, completed = 0;

    // Initial check for procs arrived at time 0
    for (int i = 0; i < n; i++) {
        if (res.procs[i].arrival_time <= current_time && !in_queue[i]) {
            queue[q_tail++] = i;
            in_queue[i] = true;
        }
    }

    while (completed < n) {
        if (q_head == q_tail) {
            // Queue empty, jump to next arrival
            int next_arr = 1e9;
            for (int i = 0; i < n; i++) {
                if (!res.procs[i].is_completed && res.procs[i].arrival_time < next_arr) {
                    next_arr = res.procs[i].arrival_time;
                }
            }
            add_gantt_slice(&res, -1, current_time, next_arr);
            current_time = next_arr;
            for (int i = 0; i < n; i++) {
                if (!res.procs[i].is_completed && res.procs[i].arrival_time <= current_time && !in_queue[i]) {
                    queue[q_tail++] = i;
                    in_queue[i] = true;
                }
            }
            continue;
        }

        int idx = queue[q_head++];
        in_queue[idx] = false;

        if (res.procs[idx].start_time == -1) {
            res.procs[idx].start_time = current_time;
            res.procs[idx].response_time = current_time - res.procs[idx].arrival_time;
        }

        int run_time = (res.procs[idx].remaining_time > quantum) ? quantum : res.procs[idx].remaining_time;
        add_gantt_slice(&res, res.procs[idx].pid, current_time, current_time + run_time);

        current_time += run_time;
        res.procs[idx].remaining_time -= run_time;

        // Check newly arrived processes during this time slice
        for (int i = 0; i < n; i++) {
            if (!res.procs[i].is_completed && i != idx && res.procs[i].arrival_time <= current_time && !in_queue[i]) {
                queue[q_tail++] = i;
                in_queue[i] = true;
            }
        }

        if (res.procs[idx].remaining_time > 0) {
            queue[q_tail++] = idx;
            in_queue[idx] = true;
        } else {
            res.procs[idx].completion_time = current_time;
            res.procs[idx].turnaround_time = res.procs[idx].completion_time - res.procs[idx].arrival_time;
            res.procs[idx].waiting_time = res.procs[idx].turnaround_time - res.procs[idx].burst_time;
            res.procs[idx].is_completed = 1;
            completed++;
        }
    }

    finalize_metrics(&res);
    return res;
}

