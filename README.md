# Project-Based Learning (PBL)

**Information Science and Engineering**  
**St Aloysius (Deemed to be University), Mangaluru**  
*Subject:* PBL | *Team:* Team 8

[![Language: C11](https://img.shields.io/badge/Language-C11-00599C.svg?style=flat-square&logo=c)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Standard: POSIX IPC](https://img.shields.io/badge/Standard-POSIX_IPC-2b2b2b.svg?style=flat-square)](https://pubs.opengroup.org/)
[![Platform: Linux / WSL2](https://img.shields.io/badge/Platform-Linux%20%7C%20WSL2-598234.svg?style=flat-square&logo=linux)](https://ubuntu.com/)
[![Team: Team 8](https://img.shields.io/badge/PBL-Team%208-blueviolet.svg?style=flat-square)](#team-8-responsibility-matrix)

---

## Team 8 Responsibility Matrix

| Member | Role | Assigned Component & Primary Files | Individual IPC Research |
| :--- | :--- | :--- | :--- |
| **Waqqas** | **Team Leader** | System Integration, Process Supervisor (`supervisor.c`), Benchmarking Suite | POSIX Shared Memory (`shm_demo.c` + PDF) |
| **Sneha** | **Student 1** | UI Frontend Process (`ui_main.c`), Interactive CLI & Protocol Framing | System V / POSIX Message Queues (`sender.c`, `receiver.c` + PDF) |
| **Siddanth** | **Student 2** | CPU Core Engine (`core_main.c` — Stack, Memory, ALU), Async Log Queue | Anonymous Pipes & Named FIFOs (`pipe_demo.c`, `fifo_*.c` + PDF) |
| **Rafath** | **Student 3** | Dedicated Logging Daemon (`logger_main.c`), Disk Log Collector | Sockets, Signals & Semaphores (`semaphore_demo.c`, `tcp_*.c` + PDF) |

---

## Project Architecture & Scope

This project implements two core systems deliverables for the OS PBL curriculum:

1. **[Week 3: CPU Process Scheduling Simulator](#1-week-3--cpu-process-scheduling-simulator)**: An interactive simulator evaluating **FCFS, SJF (Preemptive & Non-Preemptive), Priority (Preemptive & Non-Preemptive), and Round Robin** with automated ASCII Gantt charts and performance analysis across 3 test datasets.
2. **[Week 4: Multi-Process CPU Simulator with POSIX IPC](#2-week-4--multi-process-simulator-with-posix-ipc)**: A modular system decoupling a single-process CPU simulator into **three independent cooperating processes** (UI, Core, Logger) communicating via POSIX Message Queues with an asynchronous in-core worker thread.

---

## 1. Week 3 — CPU Process Scheduling Simulator

### Objective
> *"Design and implement a process scheduling simulator that can simulate different CPU scheduling algorithms and analyze their performance metrics."*

The simulator accepts process sets defined by **Arrival Time**, **Burst Time**, and **Priority**, schedules them under configurable policies, and computes **Waiting Time (WT)**, **Turnaround Time (TAT)**, **Response Time (RT)**, and **CPU Utilization**.

### Supported Scheduling Algorithms
* **FCFS** (First-Come-First-Served)
* **SJF Non-Preemptive** (Shortest Job First)
* **SRTF Preemptive** (Shortest Remaining Time First)
* **Priority Scheduling Non-Preemptive** (Lower integer = higher priority)
* **Priority Scheduling Preemptive**
* **Round Robin (RR)** with configurable time quantum ($q$)

### Benchmark Test Datasets

<details open>
<summary><b>Click to expand built-in test case tables</b></summary>

| Dataset | Description | Process Attributes (PID, Arrival, Burst, Priority) |
| :--- | :--- | :--- |
| **Test Case 1** *(Default)* | Staggered arrival times with mixed burst | **P1**:(0, 8, 3) \| **P2**:(1, 4, 1) \| **P3**:(2, 9, 4) \| **P4**:(3, 5, 2) |
| **Test Case 2** | Mixed priority competition | **P1**:(0, 5, 2) \| **P2**:(1, 3, 1) \| **P3**:(2, 8, 4) \| **P4**:(3, 6, 3) |
| **Test Case 3** | Simultaneous arrival workload ($t = 0$) | **P1**:(0, 10, 3) \| **P2**:(0, 5, 1) \| **P3**:(0, 2, 4) \| **P4**:(0, 4, 2) |

</details>

---

### Sample Execution Output & Gantt Chart (Test Case 1)

#### SRTF (Preemptive SJF)
```text
====================================================================
 Results for Algorithm: 3. Shortest Remaining Time First (SRTF Preemptive)
====================================================================
 PID | Arrival | Burst | Priority | Start | Compl | Wait | TAT | Resp 
-----+---------+-------+----------+-------+-------+------+-----+------
 1   | 0       | 8     | 3        | 0     | 17    | 9    | 17  | 0    
 2   | 1       | 4     | 1        | 1     | 5     | 0    | 4   | 0    
 3   | 2       | 9     | 4        | 17    | 26    | 15   | 24  | 15   
 4   | 3       | 5     | 2        | 5     | 10    | 2    | 7   | 2    
-----+---------+-------+----------+-------+-------+------+-----+------
 Average Waiting Time    : 6.50 ms
 Average Turnaround Time : 13.00 ms
 Average Response Time   : 4.25 ms
 CPU Utilization         : 100.00 %

Gantt Chart:
+-------+-------+-------+-------+-------+
|  P1   |  P2   |  P4   |  P1   |  P3   |
+-------+-------+-------+-------+-------+
0      1       5       10      17      26      
```

#### Round Robin ($q = 3$)
```text
Gantt Chart:
+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+
|  P1   |  P2   |  P3   |  P4   |  P1   |  P2   |  P3   |  P4   |  P1   |  P3   |
+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+
0      3       6       9       12      15      16      19      21      23      26      
```

---

### Comparative Performance Analysis (Test Case 1)

| Algorithm | Avg Waiting Time (ms) | Avg Turnaround Time (ms) | Avg Response Time (ms) | CPU Utilization |
| :--- | :---: | :---: | :---: | :---: |
| **FCFS** | 8.75 | 15.25 | 8.75 | 100.00% |
| **SJF (Non-Preemptive)** | 7.75 | 14.25 | 7.75 | 100.00% |
| **SRTF (Preemptive)** | **6.50** | **13.00** | 4.25 | 100.00% |
| **Priority (Non-Preemptive)** | 7.75 | 14.25 | 7.75 | 100.00% |
| **Priority (Preemptive)** | **6.50** | **13.00** | 4.25 | 100.00% |
| **Round Robin ($q=3$)** | 13.50 | 20.00 | **3.00** | 100.00% |

#### Observations
1. **SRTF (Preemptive SJF)** achieved the lowest Average Waiting Time (**6.50 ms**) and Turnaround Time (**13.00 ms**) by preventing longer jobs from holding the CPU when shorter tasks arrive.
2. **Round Robin** introduced higher waiting and turnaround times due to frequent time slicing, but delivered the fastest Average Response Time (**3.00 ms**), demonstrating why time-sharing operating systems prefer Round Robin for interactive responsiveness.

---

## 2. Week 4 — Multi-Process Simulator with POSIX IPC

### Objective
> *"Enhance the existing simulator by separating it into three independent processes and enabling communication between them using an appropriate POSIX IPC mechanism."*

The architecture decomposes the monolithic baseline (`baseline_v1/simulator_v1.c`) into three discrete POSIX processes coordinated by an integration supervisor:

```mermaid
graph TD
    subgraph UI_Proc ["1. UI Process (Client — Student 1)"]
        CLI["CLI Prompt (SIM>)"]
        UIRx["IPC Response Listener"]
        UITx["IPC Request Dispatcher"]
    end

    subgraph Core_Proc ["2. Core Engine Process (Server — Student 2)"]
        CoreRx["IPC Command Receiver"]
        Dispatch["Instruction Dispatcher"]
        subgraph Subsystems ["Engine Subsystems"]
            ALU["ALU (ADD)"]
            Stack["Hardware Stack"]
            Queue["Instruction Queue"]
            Mem["Memory (256 Words)"]
        end
        WorkerThread["Async Log Worker Thread"]
        CoreTxUI["UI Reply Channel"]
        CoreTxLog["Log IPC Dispatcher"]
    end

    subgraph Logger_Proc ["3. Logging Process (Daemon — Student 3)"]
        LogRx["IPC Log Listener"]
        DiskBuffer["Line-Buffered Stream"]
        LogFile[("simulator.log")]
    end

    subgraph Supervisor ["0. Integration Supervisor (Team Leader)"]
        Orch["Fork & Exec Supervisor"]
        Signal["SIGINT / Exit Signal Handler"]
        IPC_Init["IPC Init & Cleanup"]
    end

    CLI --> UITx
    UITx -->|/pbl8_ui_to_core| CoreRx
    CoreRx --> Dispatch
    Dispatch --> ALU & Stack & Queue & Mem
    Dispatch --> CoreTxUI
    CoreTxUI -->|/pbl8_core_to_ui| UIRx
    UIRx --> CLI

    Dispatch --> WorkerThread
    WorkerThread --> CoreTxLog
    CoreTxLog -->|/pbl8_to_logger| LogRx
    LogRx --> DiskBuffer --> LogFile

    Supervisor -.->|Spawns & Supervises| UI_Proc
    Supervisor -.->|Spawns & Supervises| Core_Proc
    Supervisor -.->|Spawns & Supervises| Logger_Proc
```

---

### Binary Packet Layout & Protocol Frame

Defined in [src_v2/include/protocol.h](file:///c:/Users/innoc/Desktop/Aloy/temp2/PBL/src_v2/include/protocol.h):

```text
+-------------------+--------------------+--------------------+-----------------------+
|  uint32_t req_id  |  uint8_t opcode    |   int32_t operand  |    pid_t sender_pid   |
|    (Sequence ID)  |  (PUSH, POP, ADD)  |    (Value arg)     |      (Caller PID)     |
+-------------------+--------------------+--------------------+-----------------------+
```

### Execution Flow & Teardown Lifecycle
```mermaid
sequenceDiagram
    autonumber
    actor User
    participant UI as UI Process
    participant Core as Core Process
    participant Logger as Logger Process
    participant Disk as simulator.log

    User->>UI: Types "PUSH 42"
    UI->>Core: Sends ipc_req_t (/pbl8_ui_to_core)
    Core->>Core: Executes PUSH on stack
    par In-Core Asynchronous Logging Thread
        Core->>Logger: Sends ipc_log_t (/pbl8_to_logger)
        Logger->>Disk: Appends timestamped log entry
    and Immediate UI Response
        Core->>UI: Sends ipc_resp_t (/pbl8_core_to_ui)
    end
    UI->>User: Displays "-> Stack: pushed 42 (sp=0)"
```

---

### Empirical Benchmark: Single-Process vs. Multi-Process

Both implementations were evaluated against an automated workload of **1,000 instructions** (`PUSH`, `POP`, `ADD`, `STATUS`):

| Metric | Monolithic Baseline (`v1`) | Multi-Process POSIX IPC (`v2`) | Impact |
| :--- | :---: | :---: | :---: |
| **Total Wall Time** | `2,442.27 ms` | **`339.82 ms`** | **~7.18x Speedup** |
| **Throughput** | `409 ops/sec` | **`2,943 ops/sec`** | **+619% Throughput** |
| **Mean Latency per Op** | `2,442.27 µs` | **`339.82 µs`** | **-86.1% Latency** |
| **Logging Model** | Synchronous file I/O (`fopen`/`fclose` on every op) | Asynchronous thread ring-buffer + Daemon | CPU non-blocking |
| **Fault Isolation** | None (Shared address space) | Memory isolated (3 distinct PIDs) | High resilience |

> *For detailed trade-off justifications and benchmark methodology, see [docs/IPC_COMPARISON.md](file:///c:/Users/innoc/Desktop/Aloy/temp2/PBL/docs/IPC_COMPARISON.md) and [docs/BENCHMARKS.md](file:///c:/Users/innoc/Desktop/Aloy/temp2/PBL/docs/BENCHMARKS.md).*

---

## Quickstart & Execution Guide

### Prerequisites
* GCC 9+ with POSIX realtime (`-lrt`) and pthread (`-pthread`) support.
* Linux or WSL2 (Ubuntu 20.04 / 22.04 LTS).

### 1. Build All Targets
```bash
make all
```

### 2. Run Process Scheduling Simulator (Week 3)
```bash
./bin/scheduler_v3
```

### 3. Run Multi-Process CPU Simulator (Week 4)
```bash
./bin/simulator_v2
```
```text
SIM> PUSH 100
-> Stack: pushed 100 (sp=0)
SIM> PUSH 50
-> Stack: pushed 50 (sp=1)
SIM> ADD
-> ALU: 100 + 50 = 150 (pushed to stack)
SIM> STATUS
=== CPU STATE ===
Stack Pointer: 0
Top of Stack: 150
Instruction Count: 4
Queue Pending: 0
=================
SIM> EXIT
Core acknowledged shutdown.
```

### 4. Run Automated Benchmark Comparison
```bash
make benchmark
```

---

## Repository Directory Structure

```text
PBL/
├── README.md                          # Main project documentation & laboratory report
├── Makefile                           # Master build orchestrator
│
├── scheduler_v3/                      # Week 3: CPU Process Scheduling Simulator
│   ├── include/scheduler.h            # Algorithm headers and process structures
│   ├── src/scheduler.c                # FCFS, SJF, Priority, RR with Gantt charts
│   └── Makefile
│
├── baseline_v1/                       # Baseline: Single-Process Monolithic Simulator
│   ├── simulator_v1.c
│   └── Makefile
│
├── src_v2/                            # Week 4: Multi-Process POSIX Simulator
│   ├── include/protocol.h             # Shared IPC packet contracts & opcodes
│   ├── ui/ui_main.c                   # Student 1: UI Process
│   ├── core/core_main.c               # Student 2: Core Engine with async log thread
│   ├── logger/logger_main.c           # Student 3: Logging Process Daemon
│   ├── launcher/supervisor.c          # Team Leader: Integration supervisor
│   └── Makefile
│
├── research/                          # Phase 1: Individual Research & Prototypes
│   ├── waqqas_shared_memory/          # Shared Memory prototype (`shm_demo.c` + PDF)
│   ├── sneha_message_queues/          # Message Queues prototype (`sender.c`, `receiver.c` + PDF)
│   ├── siddanth_pipes_fifos/          # Pipes & FIFOs prototype (`pipe_demo.c` + PDF)
│   └── rafath_sockets_signals/        # Sockets, Signals & Semaphores prototype (`demo.c` + PDF)
│
├── benchmarks/                        # Phase 4: Performance & Benchmarking Suite
│   ├── workloads/                     # Synthetic test workloads
│   ├── generate_workload.py           # Workload generator
│   ├── run_benchmark.sh               # Execution timer & metric collector
│   └── results.txt                    # Benchmark output logs
│
└── docs/                              # Detailed engineering specifications
    ├── ARCHITECTURE.md                # System design & protocol specifications
    ├── IPC_COMPARISON.md              # In-depth analysis of all 4 IPC mechanisms
    └── BENCHMARKS.md                  # Detailed performance analysis
```
