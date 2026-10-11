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
