# Subsystem Architecture Notes: Logging Daemon & Priority Scheduling

**Engineer:** Rafath  
**Git Identity:** `moidrafath04` ([GitHub Profile](https://github.com/moidrafath04))  
**Subsystems Owned:** Dedicated Logging Daemon Process, Log Stream Management, Timestamp Formatting, Priority Scheduling Algorithms (Preemptive & Non-Preemptive)  

---

## 1. Overview & Responsibilities

Rafath was responsible for designing and implementing the asynchronous logging daemon for the multi-process simulator, managing disk stream persistence, formatting structured event telemetry, and developing the priority-based process scheduling algorithms in the CPU scheduling suite.

### Subsystem Deliverables:
1. **Dedicated Logging Daemon (`logger_proc`):** Authored [`src_v2/logger/logger_main.c`](https://github.com/sneharao1811/PBL/blob/main/src_v2/logger/logger_main.c), implementing a standalone daemon process continuously listening on `/pbl8_to_logger`.
2. **Line-Buffered File Streaming:** Configured optimized file stream buffering via `setvbuf(f, NULL, _IOLBF, 0)` targeting `simulator.log`, ensuring automatic line-buffered disk writes upon newline without requiring high-overhead `fopen`/`fclose` cycles.
3. **Structured Event Ingestion:** Ingested binary `ipc_log_t` packets, extracting log severity level (`INFO`, `WARN`, `ERROR`), source subsystem (`UI`, `CORE`, `SUPERVISOR`), and structured message payloads.
4. **Deterministic Wall-Clock Timestamps:** Implemented human-readable epoch timestamp conversion using `localtime()` and `strftime("%Y-%m-%d %H:%M:%S")`.
5. **Sentinel-Driven Daemon Shutdown:** Designed termination handling based on the supervisor's `__SHUTDOWN__` control packet, ensuring all pending logs in the IPC queue are flushed to disk before closing descriptors and exiting.
6. **Non-Preemptive Priority Scheduling Engine:** Implemented `run_priority_non_preemptive` in [`scheduler_v3/src/scheduler.c`](https://github.com/sneharao1811/PBL/blob/main/scheduler_v3/src/scheduler.c).
7. **Preemptive Priority Scheduling Engine:** Implemented `run_priority_preemptive` in [`scheduler_v3/src/scheduler.c`](https://github.com/sneharao1811/PBL/blob/main/scheduler_v3/src/scheduler.c).
8. **Sockets, Signals & Semaphores Research:** Prototyped TCP socket servers/clients, POSIX semaphores, and signal handlers in [`research/rafath_sockets_signals/tcp_server.c`](https://github.com/sneharao1811/PBL/blob/main/research/rafath_sockets_signals/tcp_server.c), `tcp_client.c`, `semaphore_demo.c`, and `signal_demo.c`, authoring the comparative research paper `IPC Through Sockets, Signals and Semaphores (with a Comparison of All IPC Methods).pdf`.

---

## 2. Source Code Inventory

| File Path | Description |
| :--- | :--- |
| [`src_v2/logger/logger_main.c`](https://github.com/sneharao1811/PBL/blob/main/src_v2/logger/logger_main.c) | Dedicated logging daemon: opens `/pbl8_to_logger`, processes structured log packets, formats timestamps, streams to `simulator.log`, and handles shutdown. |
| [`scheduler_v3/src/scheduler.c`](https://github.com/sneharao1811/PBL/blob/main/scheduler_v3/src/scheduler.c) *(Lines 220–267)* | Non-Preemptive Priority Scheduling (`run_priority_non_preemptive`) implementation. |
| [`scheduler_v3/src/scheduler.c`](https://github.com/sneharao1811/PBL/blob/main/scheduler_v3/src/scheduler.c) *(Lines 269–322)* | Preemptive Priority Scheduling (`run_priority_preemptive`) implementation. |
| [`research/rafath_sockets_signals/tcp_server.c`](https://github.com/sneharao1811/PBL/blob/main/research/rafath_sockets_signals/tcp_server.c) | Phase 1 prototype: Concurrent TCP server listening on network sockets. |
| [`research/rafath_sockets_signals/tcp_client.c`](https://github.com/sneharao1811/PBL/blob/main/research/rafath_sockets_signals/tcp_client.c) | Phase 1 prototype: TCP client transmitting data over AF_INET stream sockets. |
| [`research/rafath_sockets_signals/semaphore_demo.c`](https://github.com/sneharao1811/PBL/blob/main/research/rafath_sockets_signals/semaphore_demo.c) | Phase 1 prototype: POSIX named semaphore synchronization (`sem_open`, `sem_wait`, `sem_post`). |
| [`research/rafath_sockets_signals/signal_demo.c`](https://github.com/sneharao1811/PBL/blob/main/research/rafath_sockets_signals/signal_demo.c) | Phase 1 prototype: Process signal handling using `sigaction`. |
| [`research/rafath_sockets_signals/IPC Through Sockets, Signals and Semaphores (with a Comparison of All IPC Methods).pdf`](https://github.com/sneharao1811/PBL/tree/main/research/rafath_sockets_signals) | Comparative research paper evaluating network sockets, POSIX signals, semaphores, and overall IPC trade-offs. |

---

## 3. Engineering Decisions & Architecture Rationale

### Memory Isolation & Fault Decoupling
By isolating the logger into a dedicated process ([`src_v2/logger/logger_main.c`](https://github.com/sneharao1811/PBL/blob/main/src_v2/logger/logger_main.c)), physical disk stalls or crashes do not impact the core simulation engine. In `core_main.c`, when the Core background thread sends log messages, it calls `mq_timedsend()` with a 20ms timeout. Even if the Logger process experiences high disk I/O pressure, the Core execution engine continues executing instructions uninterrupted.

### Line-Buffered Stream Persistence
In naive implementations, logging to disk during loop execution involves repeatedly calling `fopen()` and `fclose()` to ensure data reaches disk. This introduces massive overhead due to file descriptor allocation and directory index lookups:
```c
FILE *f = fopen(LOG_FILE_PATH, "a");
setvbuf(f, NULL, _IOLBF, 0); // Automatic line buffering
```
With `setvbuf(_IOLBF)`, the file descriptor remains open in memory, and the C runtime flushes the internal write buffer automatically upon each newline character (`\n`), providing high throughput while preventing data loss.

### Priority Scheduling Dynamics & Aging
In [`scheduler_v3/src/scheduler.c`](https://github.com/sneharao1811/PBL/blob/main/scheduler_v3/src/scheduler.c):
- **Non-Preemptive Priority:** Processes with the lowest priority integer execute to completion once scheduled.
- **Preemptive Priority:** If a task with a higher priority (e.g. priority 1 vs priority 3) arrives at time $t$, the scheduler preempts the currently running task, recording preemption intervals on the Gantt timeline.
- **Starvation Considerations:** The implementation demonstrated the primary limitation of static priority scheduling—potential starvation of lower-priority processes—which production operating systems solve using priority aging.

---

## 4. Git Commit History

```text
9607ee1 | 2026-10-12 | feat(scheduler): implement priority scheduling policies
2ee58ca | 2026-10-09 | feat(logger): implement dedicated logging daemon
e5d97d8 | 2026-10-01 | research: add sockets, signals, and semaphores documentation
```
