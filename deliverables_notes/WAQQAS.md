# Subsystem Architecture Notes: System Integration, Supervisor, Benchmarking & Deadlock Architecture

**Engineer:** Waqqas  
**Git Identity:** `Waqqas H` / `w4qq4s` ([GitHub Profile](https://github.com/w4qq4s))  
**Subsystems Owned:** Process Lifecycle Supervisor, POSIX IPC Protocol Definition, Baseline Monolithic Simulator, Automated Benchmarking Suite, Scheduler Visualization & Test Engine, Deadlock System Architecture & Core State Models

---

## 1. Overview & Responsibilities

Waqqas served as the integration lead and systems architect across the project, responsible for designing the IPC protocol contracts, supervising process lifecycles, establishing the build automation framework, engineering empirical performance benchmarks, developing visualization tooling for the scheduling engine, and architecting the data structures and core state engine for the Week 4 Deadlock Simulation System.

### Subsystem Deliverables:
1. **IPC Protocol Specification & Framing:** Authored [`src_v2/include/protocol.h`](https://github.com/sneharao1811/PBL/blob/main/src_v2/include/protocol.h), defining binary packet layouts (`ipc_req_t`, `ipc_resp_t`, `ipc_log_t`), operation codes, and POSIX queue endpoints.
2. **Process Lifecycle Supervisor:** Built [`src_v2/launcher/supervisor.c`](https://github.com/sneharao1811/PBL/blob/main/src_v2/launcher/supervisor.c), orchestrating message queue allocation, child process creation (`fork`/`exec`), signal handling (`SIGINT`/`SIGTERM`), and deterministic teardown (`mq_unlink`).
3. **Monolithic Baseline Engine (v1):** Developed [`baseline_v1/simulator_v1.c`](https://github.com/sneharao1811/PBL/blob/main/baseline_v1/simulator_v1.c) to establish a synchronous execution baseline for empirical performance comparison.
4. **Benchmarking Harness & Analytics:** Designed [`benchmarks/generate_workload.py`](https://github.com/sneharao1811/PBL/blob/main/benchmarks/generate_workload.py) and [`benchmarks/run_benchmark.sh`](https://github.com/sneharao1811/PBL/blob/main/benchmarks/run_benchmark.sh), measuring wall-clock time, ops/sec throughput, and per-operation latency across 1,000 and 5,000 operation workloads.
5. **Scheduler Visualization & Test Suite:** Authored the ASCII Gantt chart renderer, process metrics summarization (`finalize_metrics`), and automated test dataset runner in [`scheduler_v3/src/scheduler.c`](https://github.com/sneharao1811/PBL/blob/main/scheduler_v3/src/scheduler.c).
6. **Deadlock State Models & Core Math Engine:** Authored [`deadlock_v4/include/deadlock.h`](https://github.com/sneharao1811/PBL/blob/main/deadlock_v4/include/deadlock.h) and [`deadlock_v4/src/deadlock_core.c`](https://github.com/sneharao1811/PBL/blob/main/deadlock_v4/src/deadlock_core.c), defining vectors and matrices ($E, A, C, M, N, Q$), need recalculation, resource headroom validation, benchmark presets, and telemetry formatters.
7. **Shared Memory IPC Research:** Researched POSIX Shared Memory (`shm_open`, `mmap`, `shm_unlink`) in [`research/waqqas_shared_memory/shm_demo.c`](https://github.com/sneharao1811/PBL/blob/main/research/waqqas_shared_memory/shm_demo.c) and authored the technical paper `Inter Process Communication (IPC) - Fundamentals and Shared Memory.pdf`.

---

## 2. Source Code Inventory

| File Path | Description |
| :--- | :--- |
| [`src_v2/launcher/supervisor.c`](https://github.com/sneharao1811/PBL/blob/main/src_v2/launcher/supervisor.c) | Master process supervisor: allocates queues, spawns `logger_proc`, `core_proc`, `ui_proc`, coordinates teardown, and unlinks queues. |
| [`src_v2/include/protocol.h`](https://github.com/sneharao1811/PBL/blob/main/src_v2/include/protocol.h) | Shared protocol contracts: packet structures, queue paths, opcodes (`CMD_PUSH`, `CMD_POP`, `CMD_ADD`, `CMD_STATUS`, `CMD_EXIT`), and status codes. |
| [`src_v2/Makefile`](https://github.com/sneharao1811/PBL/blob/main/src_v2/Makefile) | Subsystem Makefile with compilation rules for all three discrete binaries and the supervisor launcher. |
| [`Makefile`](https://github.com/sneharao1811/PBL/blob/main/Makefile) | Root Makefile orchestrating `v1`, `v2`, `scheduler`, `test`, `benchmark`, and `clean` targets. |
| [`baseline_v1/simulator_v1.c`](https://github.com/sneharao1811/PBL/blob/main/baseline_v1/simulator_v1.c) | Single-process baseline simulator used to quantify multi-process speedup. |
| [`baseline_v1/Makefile`](https://github.com/sneharao1811/PBL/blob/main/baseline_v1/Makefile) | Makefile for compiling the baseline monolithic executable. |
| [`benchmarks/generate_workload.py`](https://github.com/sneharao1811/PBL/blob/main/benchmarks/generate_workload.py) | Python generator creating balanced sequences of CPU instructions for stress testing. |
| [`benchmarks/run_benchmark.sh`](https://github.com/sneharao1811/PBL/blob/main/benchmarks/run_benchmark.sh) | Execution timer and comparative benchmark report generator. |
| [`benchmarks/results.txt`](https://github.com/sneharao1811/PBL/blob/main/benchmarks/results.txt) | Captured benchmark execution metrics comparing v1 and v2. |
| [`docs/ARCHITECTURE.md`](https://github.com/sneharao1811/PBL/blob/main/docs/ARCHITECTURE.md) | Architectural specification, process topologies, packet byte layouts, and sequence diagrams. |
| [`docs/IPC_COMPARISON.md`](https://github.com/sneharao1811/PBL/blob/main/docs/IPC_COMPARISON.md) | Quantitative and qualitative engineering analysis comparing all 4 evaluated IPC mechanisms. |
| [`docs/BENCHMARKS.md`](https://github.com/sneharao1811/PBL/blob/main/docs/BENCHMARKS.md) | Technical report on benchmark methodology, I/O bottleneck analysis, and multi-process throughput. |
| [`scheduler_v3/include/scheduler.h`](https://github.com/sneharao1811/PBL/blob/main/scheduler_v3/include/scheduler.h) | Process control block struct, Gantt interval records, and scheduling algorithm signatures. |
| [`scheduler_v3/Makefile`](https://github.com/sneharao1811/PBL/blob/main/scheduler_v3/Makefile) | Build script for the CPU scheduling simulation engine. |
| [`scheduler_v3/src/scheduler.c`](https://github.com/sneharao1811/PBL/blob/main/scheduler_v3/src/scheduler.c) *(Gantt/Runner)* | Gantt chart interval merger (`add_gantt_slice`), ASCII renderer, metric calculations, and automated dataset test runner. |
| [`deadlock_v4/include/deadlock.h`](https://github.com/sneharao1811/PBL/blob/main/deadlock_v4/include/deadlock.h) | Central header: `deadlock_sys_t`, `safety_result_t`, `detection_result_t`, RAG edge types, all function prototypes. |
| [`deadlock_v4/src/deadlock_core.c`](https://github.com/sneharao1811/PBL/blob/main/deadlock_v4/src/deadlock_core.c) | State initialization, matrix recalculation ($N = M - C$, $A = E - \sum C$), 3 preset workloads, telemetry formatters. |
| [`deadlock_v4/deadlock_sim.py`](https://github.com/sneharao1811/PBL/blob/main/deadlock_v4/deadlock_sim.py) | Complete standalone Python reimplementation with algorithm parity for cross-platform CLI execution. |
| [`deadlock_v4/gui/server.py`](https://github.com/sneharao1811/PBL/blob/main/deadlock_v4/gui/server.py) | Zero-dependency HTTP server with automatic browser launch for GUI delivery. |
| [`deadlock_v4/Makefile`](https://github.com/sneharao1811/PBL/blob/main/deadlock_v4/Makefile) | Build script for the native C deadlock CLI binary. |
| [`docs/DEADLOCK_ARCHITECTURE.md`](https://github.com/sneharao1811/PBL/blob/main/docs/DEADLOCK_ARCHITECTURE.md) | Complete architectural specification with mermaid diagrams, mathematical models, and GUI architecture. |
| [`research/waqqas_shared_memory/shm_demo.c`](https://github.com/sneharao1811/PBL/blob/main/research/waqqas_shared_memory/shm_demo.c) | Working prototype demonstrating POSIX shared memory creation, truncation, mapping, and synchronization. |
| [`research/waqqas_shared_memory/Inter Process Communication (IPC) - Fundamentals and Shared Memory.pdf`](https://github.com/sneharao1811/PBL/tree/main/research/waqqas_shared_memory) | Research paper on shared memory architectures, kernel virtual memory mapping, and cache coherence. |

---

## 3. Engineering Decisions & Architecture Rationale

### Choice of POSIX Message Queues over Shared Memory
During Phase 1 research, Shared Memory (`shm_open`, `mmap`) proved capable of zero-copy transfers. However, for a CPU simulator exchanging discrete 32-byte RPC command packets, shared memory requires implementing cross-process mutexes or semaphores (`PTHREAD_PROCESS_SHARED`) inside mapped RAM, which significantly increases code complexity and deadlock risk. POSIX Message Queues (`<mqueue.h>`) provide atomic kernel-managed synchronization, discrete message boundaries (eliminating manual byte framing), and priority queueing out of the box.

### Deterministic Lifecycle & Signal Handling
In [`src_v2/launcher/supervisor.c`](https://github.com/sneharao1811/PBL/blob/main/src_v2/launcher/supervisor.c), message queues must never be left dangling in `/dev/mqueue/` if a user terminates execution prematurely:
- A signal handler intercepts `SIGINT` and `SIGTERM`, sending `SIGTERM` to all active child PIDs and unlinking the queues immediately (`mq_unlink`).
- During normal termination, an orderly teardown sequence is observed: the UI process exits first -> supervisor terminates Core -> supervisor sends the `__SHUTDOWN__` control packet to the Logger -> Logger flushes its disk stream and exits -> supervisor unlinks the queues.

### Eliminating the Monolithic I/O Bottleneck
In the baseline simulator ([`baseline_v1/simulator_v1.c`](https://github.com/sneharao1811/PBL/blob/main/baseline_v1/simulator_v1.c)), logging was invoked synchronously on the CPU execution path (`fopen("simulator.log", "a")` + `fclose()` per instruction), causing execution to stall on disk writes (averaging `2,442 ms` for 1,000 instructions). The multi-process architecture decouples CPU operations from disk I/O, allowing instructions to execute in memory while logging is batched asynchronously by a dedicated daemon, achieving `339 ms` (**~7.18x speedup**).

---

## 4. Git Commit History

```text
c517012 | 2026-10-23 | docs: update laboratory report and student deliverables for deadlock simulation
f388fb7 | 2026-10-23 | feat(deadlock): integrate build orchestration, standalone runner, and test suite
11a7242 | 2026-10-18 | feat(deadlock): initialize state models, headers, and core matrix math
9211006 | 2026-10-14 | docs: student-specific documentation added
6246213 | 2026-10-14 | docs: add SOE to readme.md
25c711d | 2026-10-12 | perf: add synthetic workload generator and benchmark comparison suite
411290c | 2026-10-12 | feat(scheduler): add ASCII Gantt chart renderer, test suites, and comparison table
1d0b5d2 | 2026-10-11 | feat(scheduler): initialize process models, headers, and metrics framework
c6af8b6 | 2026-10-10 | feat(supervisor): process lifecycle management and build orchestration
793c039 | 2026-10-05 | arch: define shared POSIX IPC protocol and message headers
296e4f6 | 2026-10-03 | feat: single-process baseline simulator v1
6c43897 | 2026-09-30 | research: add shared memory fundamentals and demo
```
