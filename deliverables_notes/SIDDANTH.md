# Subsystem Architecture Notes: Core CPU Engine & SJF/SRTF Scheduling

**Engineer:** Siddanth  
**Git Identity:** `mallyasiddanth` ([GitHub Profile](https://github.com/mallyasiddanth))  
**Subsystems Owned:** CPU Core Engine Process, Hardware Subsystems (Stack, Memory, ALU, Queue), Multithreaded Asynchronous Logging Worker, Shortest Job First (SJF) & Shortest Remaining Time First (SRTF) Scheduling Algorithms, Deadlock Avoidance Engine & Banker's Safety Algorithm

---

## 1. Overview & Responsibilities

Siddanth was responsible for designing and implementing the execution engine of the multi-process CPU simulator—managing hardware simulation state, memory, stack pointer mechanics, instruction queueing, arithmetic operations, and internal multithreaded concurrency. In the scheduling simulator, he authored the greedy and preemptive burst-optimal scheduling algorithms. For the Deadlock Simulation System, he engineered the Deadlock Avoidance engine using Banker's Algorithm, Safe Sequence evaluation, and dynamic Resource-Request validation.

### Subsystem Deliverables:
1. **Core CPU Engine Process (`core_proc`):** Authored [`src_v2/core/core_main.c`](https://github.com/sneharao1811/PBL/blob/main/src_v2/core/core_main.c), implementing CPU state management, IPC instruction receipt, and execution dispatching.
2. **Simulated Hardware Subsystems:**
   - **Memory Subsystem:** Simulated 256-word physical RAM storage (`int memory[MEM_SIZE]`).
   - **Hardware Stack Subsystem:** Simulated 64-word stack (`int stack[STACK_SIZE]`, stack pointer `sp`) with boundary validation.
   - **Instruction Queue Subsystem:** Implemented a circular FIFO queue (`inst_queue`, `enqueue_instruction`, `dequeue_instruction`) buffering incoming requests prior to execution.
   - **ALU Subsystem:** Handled arithmetic operations for `CMD_ADD` with operand validation.
3. **Multithreaded Asynchronous Logging Worker:** Implemented an in-core worker thread using POSIX threads (`pthread_create`, `pthread_mutex_t`, `pthread_cond_t`) that buffers log messages into a memory ring-buffer and asynchronously flushes them to the logger daemon via `mq_timedsend()`.
4. **Error Detection & Status Code Generation:** Enforced defensive bounds checks for stack overflow (`STATUS_ERR_OVERFLOW`), underflow (`STATUS_ERR_UNDERFLOW`), and operand shortage (`STATUS_ERR_OPERANDS`).
5. **SJF Non-Preemptive Scheduling Engine:** Implemented `run_sjf_non_preemptive` in [`scheduler_v3/src/scheduler.c`](https://github.com/sneharao1811/PBL/blob/main/scheduler_v3/src/scheduler.c).
6. **SRTF Preemptive Scheduling Engine:** Implemented `run_srtf_preemptive` in [`scheduler_v3/src/scheduler.c`](https://github.com/sneharao1811/PBL/blob/main/scheduler_v3/src/scheduler.c), supporting cycle-by-cycle preemption.
7. **Banker's Deadlock Avoidance Engine:** Authored [`deadlock_v4/src/banker.c`](https://github.com/sneharao1811/PBL/blob/main/deadlock_v4/src/banker.c), implementing the Banker's Safety Algorithm to discover Safe Sequences $\langle P_i \dots \rangle$ and the Resource-Request algorithm to test hypothetical state transitions and avoid unsafe operations.
8. **Pipes and FIFOs Research:** Prototyped anonymous pipes and named FIFOs in [`research/siddanth_pipes_fifos/pipe_demo.c`](https://github.com/sneharao1811/PBL/blob/main/research/siddanth_pipes_fifos/pipe_demo.c), `fifo_writer.c`, and `fifo_reader.c`, authoring the technical report `IPC(Through Pipes and FIFOs).pdf`.

---

## 2. Source Code Inventory

| File Path | Description |
| :--- | :--- |
| [`src_v2/core/core_main.c`](https://github.com/sneharao1811/PBL/blob/main/src_v2/core/core_main.c) | Complete Core Engine: IPC queue listeners, instruction queueing, stack/memory/ALU execution, asynchronous pthread logging worker, and response dispatch. |
| [`scheduler_v3/src/scheduler.c`](https://github.com/sneharao1811/PBL/blob/main/scheduler_v3/src/scheduler.c) *(Lines 100–145)* | Shortest Job First Non-Preemptive (`run_sjf_non_preemptive`) scheduling algorithm implementation. |
| [`scheduler_v3/src/scheduler.c`](https://github.com/sneharao1811/PBL/blob/main/scheduler_v3/src/scheduler.c) *(Lines 147–218)* | Shortest Remaining Time First Preemptive (`run_srtf_preemptive`) scheduling algorithm implementation. |
| [`research/siddanth_pipes_fifos/pipe_demo.c`](https://github.com/sneharao1811/PBL/blob/main/research/siddanth_pipes_fifos/pipe_demo.c) | Phase 1 prototype: Anonymous pipe communication across parent and child processes. |
| [`research/siddanth_pipes_fifos/fifo_writer.c`](https://github.com/sneharao1811/PBL/blob/main/research/siddanth_pipes_fifos/fifo_writer.c) | Phase 1 prototype: Named FIFO writer process using POSIX `mkfifo`. |
| [`research/siddanth_pipes_fifos/fifo_reader.c`](https://github.com/sneharao1811/PBL/blob/main/research/siddanth_pipes_fifos/fifo_reader.c) | Phase 1 prototype: Named FIFO reader process ingesting byte streams. |
| [`research/siddanth_pipes_fifos/IPC(Through Pipes and FIFOs).pdf`](https://github.com/sneharao1811/PBL/tree/main/research/siddanth_pipes_fifos) | Technical research report evaluating byte stream semantics, FIFO buffer thresholds, and pipe descriptor mechanics. |

---

## 3. Engineering Decisions & Architecture Rationale

### Asynchronous Producer-Consumer Logging Worker
In [`src_v2/core/core_main.c`](https://github.com/sneharao1811/PBL/blob/main/src_v2/core/core_main.c), invoking IPC transmissions directly during CPU execution would introduce IPC queue backpressure and disk latency into the simulation loop:
```c
#define LOG_RING_SIZE 64
static ipc_log_t log_ring[LOG_RING_SIZE];
static int ring_head = 0, ring_tail = 0, ring_count = 0;
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  log_cond  = PTHREAD_COND_INITIALIZER;
static pthread_t log_thread;
```
1. **Producer (`queue_async_log`):**
   When an instruction completes (e.g. `PUSH 42`), the Core engine locks `log_mutex`, enqueues a log item into `log_ring`, signals `log_cond`, and immediately unlocks the mutex. CPU execution completes in nanoseconds.
2. **Consumer (`logger_worker_thread`):**
   A detached background thread wakes up on `pthread_cond_wait`, pulls log items from the ring buffer, and transmits them across `/pbl8_to_logger` via `mq_timedsend()`.
3. **Graceful Draining:**
   On shutdown, `core_main.c` broadcasts on `log_cond`, drains remaining entries in `log_ring`, and joins the worker thread (`pthread_join`) before unlinking descriptors.

### Defensive Stack Pointer Boundary Enforcement
The hardware stack enforces strict runtime boundaries ($0 \le sp < 64$):
- Pushing when $sp=63$ aborts the operation, sets `STATUS_ERR_OVERFLOW` in the response packet, and logs an ERROR event.
- Popping when $sp=-1$ aborts execution, sets `STATUS_ERR_UNDERFLOW`, and alerts the caller.
- `CMD_ADD` checks that $sp \ge 1$ before popping operands. This defensive model prevents memory corruption and provides deterministic error messaging across process boundaries.

### Algorithmic Comparison: Non-Preemptive SJF vs Preemptive SRTF
In [`scheduler_v3/src/scheduler.c`](https://github.com/sneharao1811/PBL/blob/main/scheduler_v3/src/scheduler.c), empirical testing validated that:
- Non-Preemptive SJF avoids preemption overhead, but suffers when a long job arrives first ($t=0$).
- Preemptive SRTF achieves the lowest Average Waiting Time (**6.50 ms**) and Turnaround Time (**13.00 ms**) in Test Case 1 by preempting long jobs immediately upon the arrival of shorter tasks.

---

## 4. Git Commit History

```text
8bf0b7b | 2026-10-11 | feat(scheduler): implement SJF non-preemptive and SRTF preemptive scheduling
8d818df | 2026-10-08 | feat(core): implement CPU engine with ALU, stack, queue, and async worker
d630666 | 2026-09-30 | research: add pipes and fifos implementation
```
