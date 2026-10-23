#!/usr/bin/env python3
"""
Deadlock Simulation System - Pure Python Standalone CLI & Simulation Engine
Matches the exact 12-item menu and algorithm specifications of the C implementation.

OS Project-Based Learning (PBL) - Team 8
School of Engineering, St Aloysius (Deemed to be University)
Lead: Waqqas (Integration & Architecture)
Algorithmic Implementations: Siddanth (Banker's Avoidance), Rafath (Detection & Recovery), Sneha (CLI & RAG)
"""

import sys
import copy

class DeadlockSystem:
    def __init__(self):
        self.num_processes = 0
        self.num_resources = 0
        self.process_names = []
        self.resource_names = []
        self.total_resources = []
        self.available = []
        self.allocation = []
        self.maximum = []
        self.need = []
        self.request = []
        self.process_active = []
        self.load_textbook_preset()

    def reset_matrices(self):
        self.process_active = [True] * self.num_processes
        self.allocation = [[0] * self.num_resources for _ in range(self.num_processes)]
        self.maximum = [[0] * self.num_resources for _ in range(self.num_processes)]
        self.need = [[0] * self.num_resources for _ in range(self.num_processes)]
        self.request = [[0] * self.num_resources for _ in range(self.num_processes)]
        self.available = list(self.total_resources)

    def recalculate_need(self):
        self.need = [[0] * self.num_resources for _ in range(self.num_processes)]
        for i in range(self.num_processes):
            if not self.process_active[i]:
                continue
            for j in range(self.num_resources):
                self.need[i][j] = max(0, self.maximum[i][j] - self.allocation[i][j])

    def recalculate_available(self):
        self.available = [0] * self.num_resources
        for j in range(self.num_resources):
            allocated_sum = sum(self.allocation[i][j] for i in range(self.num_processes) if self.process_active[i])
            self.available[j] = max(0, self.total_resources[j] - allocated_sum)

    def load_textbook_preset(self):
        self.num_processes = 5
        self.num_resources = 3
        self.process_names = ["P0", "P1", "P2", "P3", "P4"]
        self.resource_names = ["A (CPU)", "B (RAM)", "C (Disk)"]
        self.total_resources = [10, 5, 7]
        self.process_active = [True] * 5

        self.allocation = [
            [0, 1, 0],
            [2, 0, 0],
            [3, 0, 2],
            [2, 1, 1],
            [0, 0, 2]
        ]
        self.maximum = [
            [7, 5, 3],
            [3, 2, 2],
            [9, 0, 2],
            [2, 2, 2],
            [4, 3, 3]
        ]
        self.request = [[0, 0, 0] for _ in range(5)]
        self.recalculate_need()
        self.recalculate_available()

    def load_deadlock_preset(self):
        self.num_processes = 3
        self.num_resources = 3
        self.process_names = ["P0", "P1", "P2"]
        self.resource_names = ["R1 (Mutex)", "R2 (Tape)", "R3 (Plotter)"]
        self.total_resources = [1, 1, 1]
        self.process_active = [True] * 3

        # Circular wait: P0 holds R0 req R1; P1 holds R1 req R2; P2 holds R2 req R0
        self.allocation = [
            [1, 0, 0],
            [0, 1, 0],
            [0, 0, 1]
        ]
        self.maximum = [
            [1, 1, 0],
            [0, 1, 1],
            [1, 0, 1]
        ]
        self.request = [
            [0, 1, 0],
            [0, 0, 1],
            [1, 0, 0]
        ]
        self.recalculate_need()
        self.recalculate_available()

    def load_unsafe_preset(self):
        self.num_processes = 4
        self.num_resources = 2
        self.process_names = ["P0", "P1", "P2", "P3"]
        self.resource_names = ["GPU", "Network"]
        self.total_resources = [6, 6]
        self.process_active = [True] * 4

        self.allocation = [
            [2, 1],
            [1, 2],
            [2, 1],
            [1, 1]
        ]
        self.maximum = [
            [4, 3],
            [3, 4],
            [4, 3],
            [3, 3]
        ]
        self.request = [[0, 0] for _ in range(4)]
        self.recalculate_need()
        self.recalculate_available()

    def check_safety(self):
        work = list(self.available)
        finish = [not self.process_active[i] for i in range(self.num_processes)]
        sequence = []
        active_count = sum(1 for a in self.process_active if a)

        progress = True
        while progress and len(sequence) < active_count:
            progress = False
            for i in range(self.num_processes):
                if not finish[i] and self.process_active[i]:
                    if all(self.need[i][j] <= work[j] for j in range(self.num_resources)):
                        for j in range(self.num_resources):
                            work[j] += self.allocation[i][j]
                        finish[i] = True
                        sequence.append(i)
                        progress = True

        is_safe = (len(sequence) == active_count)
        return is_safe, sequence

    def request_resource(self, pid, req_vector):
        if not (0 <= pid < self.num_processes and self.process_active[pid]):
            return False, f"Process index {pid} is invalid or terminated."

        # Condition 1: Request <= Need
        for j in range(self.num_resources):
            if req_vector[j] > self.need[pid][j]:
                return False, f"DENIED: {self.process_names[pid]} requested {req_vector[j]} of {self.resource_names[j]}, which exceeds declared need ({self.need[pid][j]})."

        # Condition 2: Request <= Available
        for j in range(self.num_resources):
            if req_vector[j] > self.available[j]:
                self.request[pid][j] = req_vector[j]
                return False, f"BLOCKED: {self.process_names[pid]} requested {req_vector[j]} of {self.resource_names[j]}, but only {self.available[j]} currently available. Process must wait."

        # Hypothetical allocation
        temp_sys = copy.deepcopy(self)
        for j in range(self.num_resources):
            temp_sys.available[j] -= req_vector[j]
            temp_sys.allocation[pid][j] += req_vector[j]
            temp_sys.need[pid][j] -= req_vector[j]
            temp_sys.request[pid][j] = 0

        is_safe, seq = temp_sys.check_safety()
        if is_safe:
            for j in range(self.num_resources):
                self.available[j] -= req_vector[j]
                self.allocation[pid][j] += req_vector[j]
                self.need[pid][j] -= req_vector[j]
                self.request[pid][j] = 0
            seq_str = " -> ".join(self.process_names[k] for k in seq)
            return True, f"GRANTED: Banker's Avoidance approved request. Safe Sequence: < {seq_str} >."
        else:
            self.request[pid] = list(req_vector)
            return False, "DENIED BY BANKER'S AVOIDANCE: Granting this request leads to an UNSAFE state (potential deadlock). Request queued."

    def detect_deadlock(self):
        work = list(self.available)
        finish = []
        for i in range(self.num_processes):
            if not self.process_active[i]:
                finish.append(True)
            else:
                holds = any(self.allocation[i][j] > 0 for j in range(self.num_resources))
                finish.append(not holds)

        progress = True
        while progress:
            progress = False
            for i in range(self.num_processes):
                if not finish[i] and self.process_active[i]:
                    if all(self.request[i][j] <= work[j] for j in range(self.num_resources)):
                        for j in range(self.num_resources):
                            work[j] += self.allocation[i][j]
                        finish[i] = True
                        progress = True

        deadlocked = [i for i in range(self.num_processes) if self.process_active[i] and not finish[i]]
        return (len(deadlocked) > 0), deadlocked

    def recover(self, strategy):
        is_dl, deadlocked = self.detect_deadlock()
        if not is_dl:
            return "No deadlock detected; recovery skipped."

        if strategy == 1: # Terminate All
            for pid in deadlocked:
                for j in range(self.num_resources):
                    self.available[j] += self.allocation[pid][j]
                    self.allocation[pid][j] = 0
                    self.need[pid][j] = 0
                    self.request[pid][j] = 0
                self.process_active[pid] = False
            return f"RECOVERY: Terminated all {len(deadlocked)} deadlocked processes. Resources reclaimed."

        elif strategy == 2: # One by one
            # Pick victim holding highest resources
            victim = max(deadlocked, key=lambda pid: sum(self.allocation[pid]))
            v_name = self.process_names[victim]
            for j in range(self.num_resources):
                self.available[j] += self.allocation[victim][j]
                self.allocation[victim][j] = 0
                self.need[victim][j] = 0
                self.request[victim][j] = 0
            self.process_active[victim] = False
            return f"RECOVERY: Selectively aborted victim {v_name}. Released resources to pool."

        elif strategy == 3: # Preempt
            victim = deadlocked[0]
            v_name = self.process_names[victim]
            for j in range(self.num_resources):
                self.available[j] += self.allocation[victim][j]
                self.allocation[victim][j] = 0
                self.request[victim][j] = 0
            self.recalculate_need()
            return f"RECOVERY (PREEMPTION): Preempted resources from {v_name} and rolled back transactions."

    def print_matrices(self):
        print("\n" + "=" * 76)
        print("                  SYSTEM MATRICES & RESOURCE VECTORS                    ")
        print("=" * 76)
        print("Total Resources Vector (E):")
        print("  " + "  ".join(f"{self.resource_names[j]}: {self.total_resources[j]:<3}" for j in range(self.num_resources)))
        print("Available Resource Vector (A):")
        print("  " + "  ".join(f"{self.resource_names[j]}: {self.available[j]:<3}" for j in range(self.num_resources)))
        print("-" * 76)
        print(f"{'Process':<8} | {'Allocation (C)':<16} | {'Maximum (M)':<16} | {'Need (N)':<16} | Status")
        print("-" * 76)
        for i in range(self.num_processes):
            alloc_s = " ".join(str(x) for x in self.allocation[i])
            max_s = " ".join(str(x) for x in self.maximum[i])
            need_s = " ".join(str(x) for x in self.need[i])
            status_s = "ACTIVE" if self.process_active[i] else "TERMINATED"
            print(f"{self.process_names[i]:<8} | {alloc_s:<16} | {max_s:<16} | {need_s:<16} | {status_s}")
        print("=" * 76 + "\n")

    def print_rag(self):
        print("\n" + "=" * 76)
        print("                  RESOURCE ALLOCATION GRAPH (RAG)                      ")
        print("=" * 76)
        print("Nodes:")
        active_p = [self.process_names[i] for i in range(self.num_processes) if self.process_active[i]]
        print(f"  * Processes (Circles)  : {' '.join(f'[{p}]' for p in active_p)}")
        print(f"  * Resources (Rectangles): {' '.join(f'[{self.resource_names[j]}: {self.total_resources[j]} units]' for j in range(self.num_resources))}")
        print("\nDirected Graph Edges:")
        print("  Type         | From                  | To                    | Instances")
        print("-" * 76)

        edges = []
        for i in range(self.num_processes):
            if not self.process_active[i]: continue
            for j in range(self.num_resources):
                if self.allocation[i][j] > 0:
                    edges.append(("ASSIGNMENT", f"Resource {self.resource_names[j]}", f"Process {self.process_names[i]}", self.allocation[i][j]))
                if self.request[i][j] > 0:
                    edges.append(("REQUEST (REQ)", f"Process {self.process_names[i]}", f"Resource {self.resource_names[j]}", self.request[i][j]))

        if not edges:
            print("  (No active allocation or request edges in current graph)")
        else:
            for etype, src, dst, cnt in edges:
                print(f"  {etype:<12} | {src:<21} | {dst:<21} | {cnt}")

        is_dl, deadlocked = self.detect_deadlock()
        print("-" * 76)
        if is_dl:
            print(f"  Graph Status : CIRCULAR WAIT CYCLE DETECTED -> DEADLOCK CONFIRMED (PIDs: {' '.join(self.process_names[k] for k in deadlocked)})")
        else:
            print("  Graph Status : Acyclic / No Permanent Deadlock Cycle")
        print("=" * 76 + "\n")

    def print_status(self):
        is_safe, seq = self.check_safety()
        is_dl, deadlocked = self.detect_deadlock()

        print("\n" + "=" * 76)
        print("                         SYSTEM STATUS TELEMETRY                        ")
        print("=" * 76)
        print(f"Total Processes : {sum(self.process_active)} active, {self.num_processes} total")
        print(f"Resource Types  : {self.num_resources}")

        if is_dl:
            print("State Indicator : [ DEADLOCKED ]")
            print(f"Deadlocked PIDs : {' '.join(self.process_names[k] for k in deadlocked)}")
        elif is_safe:
            print("State Indicator : [ SAFE ]")
            print(f"Safe Sequence   : < {' -> '.join(self.process_names[k] for k in seq)} >")
        else:
            print("State Indicator : [ UNSAFE ]")

        print("\nResource Utilization Breakdown:")
        for j in range(self.num_resources):
            in_use = self.total_resources[j] - self.available[j]
            pct = (in_use / self.total_resources[j] * 100) if self.total_resources[j] > 0 else 0
            print(f"  * {self.resource_names[j]:<14}: Total={self.total_resources[j]:<2} | Allocated={in_use:<2} | Free={self.available[j]:<2} | Utilized={pct:5.1f}%")
        print("=" * 76 + "\n")

def main():
    sys_model = DeadlockSystem()
    print("\n[System Initialized with Standard 5-Process, 3-Resource Benchmark Scenario]\n")

    while True:
        print("========================================")
        print("       DEADLOCK SIMULATION SYSTEM       ")
        print("========================================")
        print(" 1. Create Processes")
        print(" 2. Create Resources")
        print(" 3. Enter Allocation Matrix")
        print(" 4. Enter Maximum Matrix")
        print(" 5. Display Need Matrix")
        print(" 6. Display Resource Allocation Graph")
        print(" 7. Request Resources")
        print(" 8. Run Banker's Algorithm")
        print(" 9. Detect Deadlock")
        print("10. Recover from Deadlock")
        print("11. Display System Status")
        print("12. Exit")
        print("----------------------------------------")
        print("Quick Presets: [T]extbook Safe | [D]eadlock Cycle | [U]nsafe State")

        try:
            choice = input("Enter choice: ").strip()
        except (EOFError, KeyboardInterrupt):
            print("\nExiting. Goodbye!")
            break

        if choice.upper() == 'T':
            sys_model.load_textbook_preset()
            print("\n[Loaded Standard Textbook Safe Scenario (5 processes, 3 resources)]\n")
            continue
        elif choice.upper() == 'D':
            sys_model.load_deadlock_preset()
            print("\n[Loaded Deadlocked Scenario (3 processes in circular wait)]\n")
            continue
        elif choice.upper() == 'U':
            sys_model.load_unsafe_preset()
            print("\n[Loaded High Contention Unsafe Scenario]\n")
            continue

        if choice == '1':
            try:
                np = int(input(f"Enter number of processes (1-16): "))
                if 1 <= np <= 16:
                    sys_model.num_processes = np
                    sys_model.process_names = [f"P{i}" for i in range(np)]
                    sys_model.reset_matrices()
                    print(f"Created {np} processes.\n")
            except ValueError:
                print("Invalid number.")
        elif choice == '2':
            try:
                nr = int(input(f"Enter number of resource types (1-16): "))
                if 1 <= nr <= 16:
                    sys_model.num_resources = nr
                    sys_model.resource_names = [f"R{j}" for j in range(nr)]
                    sys_model.total_resources = []
                    for j in range(nr):
                        tot = int(input(f"Enter total capacity for {sys_model.resource_names[j]}: "))
                        sys_model.total_resources.append(max(0, tot))
                    sys_model.reset_matrices()
                    print(f"Created {nr} resource types.\n")
            except ValueError:
                print("Invalid number.")
        elif choice == '3':
            print(f"Enter Allocation Matrix ({sys_model.num_processes} x {sys_model.num_resources}):")
            for i in range(sys_model.num_processes):
                raw = input(f"  {sys_model.process_names[i]} ({sys_model.num_resources} values space-separated): ").strip().split()
                for j in range(sys_model.num_resources):
                    sys_model.allocation[i][j] = int(raw[j]) if j < len(raw) else 0
            sys_model.recalculate_need()
            sys_model.recalculate_available()
            print("Allocation matrix updated.\n")
        elif choice == '4':
            print(f"Enter Maximum Claim Matrix ({sys_model.num_processes} x {sys_model.num_resources}):")
            for i in range(sys_model.num_processes):
                raw = input(f"  {sys_model.process_names[i]} ({sys_model.num_resources} values space-separated): ").strip().split()
                for j in range(sys_model.num_resources):
                    sys_model.maximum[i][j] = int(raw[j]) if j < len(raw) else 0
            sys_model.recalculate_need()
            print("Maximum matrix updated and Need matrix recalculated.\n")
        elif choice == '5':
            sys_model.recalculate_need()
            sys_model.recalculate_available()
            sys_model.print_matrices()
        elif choice == '6':
            sys_model.print_rag()
        elif choice == '7':
            try:
                pid = int(input(f"Enter requesting process index (0 to {sys_model.num_processes - 1}): "))
                raw = input(f"Enter request vector ({sys_model.num_resources} values space-separated): ").strip().split()
                req_vec = [int(raw[j]) if j < len(raw) else 0 for j in range(sys_model.num_resources)]
                ok, msg = sys_model.request_resource(pid, req_vec)
                print(f"\n>>> {msg}\n")
            except ValueError:
                print("Invalid input.")
        elif choice == '8':
            is_safe, seq = sys_model.check_safety()
            print("\n" + "=" * 76)
            print("                   BANKER'S SAFETY ALGORITHM EVALUATION                ")
            print("=" * 76)
            if is_safe:
                seq_str = " -> ".join(sys_model.process_names[k] for k in seq)
                print("Result        : SYSTEM IS IN A SAFE STATE")
                print(f"Safe Sequence : < {seq_str} >")
            else:
                print("Result        : SYSTEM IS IN AN UNSAFE STATE")
            print("=" * 76 + "\n")
        elif choice == '9':
            is_dl, deadlocked = sys_model.detect_deadlock()
            print("\n" + "=" * 76)
            print("                     DEADLOCK DETECTION ALGORITHM                      ")
            print("=" * 76)
            if is_dl:
                print("Verdict       : DEADLOCK DETECTED!")
                print(f"Deadlocked PIDs: {' '.join(sys_model.process_names[k] for k in deadlocked)}")
            else:
                print("Verdict       : NO DEADLOCK DETECTED")
            print("=" * 76 + "\n")
        elif choice == '10':
            print("\nSelect Deadlock Recovery Strategy:")
            print("  1. Terminate All Deadlocked Processes")
            print("  2. Selectively Abort Victim One-by-One (Lowest Cost / Max Reclaimed)")
            print("  3. Preempt Resources from Victim and Rollback")
            try:
                strat = int(input("Enter choice (1-3): "))
                msg = sys_model.recover(strat)
                print(f"\n>>> {msg}\n")
            except ValueError:
                print("Invalid input.")
        elif choice == '11':
            sys_model.print_status()
        elif choice == '12':
            print("\nExiting Deadlock Simulation System. Goodbye!")
            break
        else:
            print("Invalid choice. Please select 1-12.")

if __name__ == "__main__":
    main()
