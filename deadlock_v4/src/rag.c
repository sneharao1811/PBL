/**
 * @file rag.c
 * @brief Resource Allocation Graph (RAG) construction, cycle detection, ASCII and JSON export.
 * 
 * Subsystem Lead: Sneha (UI & Visualization Lead)
 * OS Project-Based Learning (PBL) - Team 8
 */

#include "../include/deadlock.h"

void rag_build_graph(const deadlock_sys_t *sys, rag_graph_t *graph) {
    if (!sys || !graph) return;
    memset(graph, 0, sizeof(rag_graph_t));

    /* Build assignment and request edges */
    for (int i = 0; i < sys->num_processes; i++) {
        if (!sys->process_active[i]) continue;

        for (int j = 0; j < sys->num_resources; j++) {
            /* Assignment edge: Resource -> Process */
            if (sys->allocation[i][j] > 0 && graph->edge_count < (int)(sizeof(graph->edges)/sizeof(graph->edges[0]))) {
                rag_edge_t *edge = &graph->edges[graph->edge_count++];
                edge->type = EDGE_ASSIGNMENT;
                edge->process_id = i;
                edge->resource_id = j;
                edge->instances = sys->allocation[i][j];
            }

            /* Request edge: Process -> Resource */
            if (sys->request[i][j] > 0 && graph->edge_count < (int)(sizeof(graph->edges)/sizeof(graph->edges[0]))) {
                rag_edge_t *edge = &graph->edges[graph->edge_count++];
                edge->type = EDGE_REQUEST;
                edge->process_id = i;
                edge->resource_id = j;
                edge->instances = sys->request[i][j];
            }
        }
    }

    /* Cycle check for single-instance or multi-instance systems */
    detection_result_t det = deadlock_detect(sys);
    graph->has_cycle = det.deadlock_detected;
}

void rag_print_ascii(const deadlock_sys_t *sys, const rag_graph_t *graph) {
    if (!sys || !graph) return;

    printf("\n========================================================================\n");
    printf("                  RESOURCE ALLOCATION GRAPH (RAG)                      \n");
    printf("========================================================================\n");
    printf("Nodes:\n");
    printf("  * Processes (Circles) : ");
    for (int i = 0; i < sys->num_processes; i++) {
        if (sys->process_active[i]) {
            printf("[%s] ", sys->process_names[i]);
        }
    }
    printf("\n  * Resources (Rectangles): ");
    for (int j = 0; j < sys->num_resources; j++) {
        printf("[%s: %d units] ", sys->resource_names[j], sys->total_resources[j]);
    }
    printf("\n\nDirected Graph Edges:\n");
    printf("  Type         | From                  | To                    | Instances\n");
    printf("---------------+-----------------------+-----------------------+----------\n");

    if (graph->edge_count == 0) {
        printf("  (No active allocation or request edges in current graph)\n");
    } else {
        for (int e = 0; e < graph->edge_count; e++) {
            const rag_edge_t *edge = &graph->edges[e];
            if (edge->type == EDGE_ASSIGNMENT) {
                printf("  ASSIGNMENT   | Resource %-12s | Process %-13s | %d\n",
                       sys->resource_names[edge->resource_id],
                       sys->process_names[edge->process_id],
                       edge->instances);
            } else {
                printf("  REQUEST (REQ)| Process %-13s | Resource %-12s | %d\n",
                       sys->process_names[edge->process_id],
                       sys->resource_names[edge->resource_id],
                       edge->instances);
            }
        }
    }
    printf("------------------------------------------------------------------------\n");
    if (graph->has_cycle) {
        printf("  \033[1;31mGraph Status : CIRCULAR WAIT CYCLE DETECTED -> DEADLOCK CONFIRMED\033[0m\n");
    } else {
        printf("  \033[1;32mGraph Status : Acyclic / No Permanent Deadlock Cycle\033[0m\n");
    }
    printf("========================================================================\n\n");
}

void rag_export_dot(const deadlock_sys_t *sys, const rag_graph_t *graph, FILE *fp) {
    if (!sys || !graph || !fp) return;

    fprintf(fp, "digraph RAG {\n");
    fprintf(fp, "    rankdir=LR;\n");
    fprintf(fp, "    node [fontname=\"Helvetica\", fontsize=11];\n");
    fprintf(fp, "    edge [fontname=\"Helvetica\", fontsize=9];\n\n");

    /* Process nodes (circles) */
    for (int i = 0; i < sys->num_processes; i++) {
        if (!sys->process_active[i]) continue;
        fprintf(fp, "    P%d [label=\"%s\", shape=circle, style=filled, fillcolor=\"#4A90D9\", fontcolor=white];\n",
                i, sys->process_names[i]);
    }

    /* Resource nodes (rectangles with instance dots) */
    for (int j = 0; j < sys->num_resources; j++) {
        fprintf(fp, "    R%d [label=\"%s\\n(%d instances)\", shape=box, style=filled, fillcolor=\"#E8A838\", fontcolor=white];\n",
                j, sys->resource_names[j], sys->total_resources[j]);
    }

    fprintf(fp, "\n");

    /* Edges */
    for (int e = 0; e < graph->edge_count; e++) {
        const rag_edge_t *edge = &graph->edges[e];
        if (edge->type == EDGE_ASSIGNMENT) {
            fprintf(fp, "    R%d -> P%d [label=\"%d\", color=\"#2ECC71\", penwidth=2.0];\n",
                    edge->resource_id, edge->process_id, edge->instances);
        } else {
            fprintf(fp, "    P%d -> R%d [label=\"%d\", color=\"%s\", style=dashed, penwidth=2.0];\n",
                    edge->process_id, edge->resource_id, edge->instances,
                    graph->has_cycle ? "#E74C3C" : "#F39C12");
        }
    }

    fprintf(fp, "}\n");
}

void rag_export_json(const deadlock_sys_t *sys, const rag_graph_t *graph, FILE *fp) {
    if (!sys || !graph || !fp) return;

    fprintf(fp, "{\n");
    fprintf(fp, "  \"num_processes\": %d,\n", sys->num_processes);
    fprintf(fp, "  \"num_resources\": %d,\n", sys->num_resources);

    /* Processes */
    fprintf(fp, "  \"processes\": [\n");
    for (int i = 0; i < sys->num_processes; i++) {
        fprintf(fp, "    {\"id\": %d, \"name\": \"%s\", \"active\": %s}%s\n",
                i, sys->process_names[i], sys->process_active[i] ? "true" : "false",
                (i == sys->num_processes - 1) ? "" : ",");
    }
    fprintf(fp, "  ],\n");

    /* Resources */
    fprintf(fp, "  \"resources\": [\n");
    for (int j = 0; j < sys->num_resources; j++) {
        fprintf(fp, "    {\"id\": %d, \"name\": \"%s\", \"total\": %d, \"available\": %d}%s\n",
                j, sys->resource_names[j], sys->total_resources[j], sys->available[j],
                (j == sys->num_resources - 1) ? "" : ",");
    }
    fprintf(fp, "  ],\n");

    /* Edges */
    fprintf(fp, "  \"edges\": [\n");
    for (int e = 0; e < graph->edge_count; e++) {
        const rag_edge_t *edge = &graph->edges[e];
        fprintf(fp, "    {\"type\": \"%s\", \"process_id\": %d, \"resource_id\": %d, \"instances\": %d}%s\n",
                (edge->type == EDGE_ASSIGNMENT) ? "assignment" : "request",
                edge->process_id, edge->resource_id, edge->instances,
                (e == graph->edge_count - 1) ? "" : ",");
    }
    fprintf(fp, "  ],\n");
    fprintf(fp, "  \"has_cycle\": %s\n", graph->has_cycle ? "true" : "false");
    fprintf(fp, "}\n");
}
