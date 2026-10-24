# Subsystem Architecture Notes: UI Process, Interactive Shell, FCFS/RR Scheduling & Deadlock GUI/RAG

**Engineer:** Sneha  
**Git Identity:** `sneharao1811` ([GitHub Profile](https://github.com/sneharao1811))  
**Subsystems Owned:** UI Frontend Process, Interactive Shell & Protocol Serialization, First-Come-First-Served (FCFS) & Round Robin (RR) Scheduling Algorithms, Deadlock 12-Option CLI & GUI Frontend, Resource Allocation Graph (RAG) Visualizer, Project Documentation

---

## 1. Overview & Responsibilities

Sneha was responsible for designing and implementing the interactive frontend process of the multi-process simulator, handling user terminal input, constructing IPC binary packets, formatting system responses, developing the core time-shared scheduling algorithms, and authoring the complete interactive CLI, Resource Allocation Graph (RAG) visualization, and Graphical User Interface (GUI) for the Week 4 Deadlock Simulation System.

### Subsystem Deliverables:
1. **Interactive UI Process (`ui_proc`):** Authored [`src_v2/ui/ui_main.c`](https://github.com/sneharao1811/PBL/blob/main/src_v2/ui/ui_main.c), implementing the user command loop, syntax parser, and IPC message dispatcher.
2. **Dual-Mode Terminal & Batch Stream Processing:** Integrated POSIX terminal detection (`isatty(fileno(stdin))`) allowing `ui_proc` to operate as an interactive prompt (`SIM> `) for manual testing, while automatically suppressing prompt banners when processing piped benchmark streams.
3. **IPC Packet Serialization:** Implemented command serialization into structured `ipc_req_t` message frames, assigning monotonic sequence IDs and caller PID tracking.
4. **FCFS Scheduling Engine:** Developed the non-preemptive First-Come-First-Served scheduling algorithm (`run_fcfs`) in [`scheduler_v3/src/scheduler.c`](https://github.com/sneharao1811/PBL/blob/main/scheduler_v3/src/scheduler.c).
5. **Round Robin Scheduling Engine:** Developed the preemptive Round Robin scheduling algorithm (`run_round_robin`) in [`scheduler_v3/src/scheduler.c`](https://github.com/sneharao1811/PBL/blob/main/scheduler_v3/src/scheduler.c), supporting configurable time quantum slices ($q$).
6. **Deadlock Interactive CLI Shell:** Authored [`deadlock_v4/src/deadlock_cli.c`](https://github.com/sneharao1811/PBL/blob/main/deadlock_v4/src/deadlock_cli.c), implementing the full 12-option menu loop for process/resource definition, matrix input, request dispatching, and status monitoring.
7. **Resource Allocation Graph (RAG) Engine:** Built [`deadlock_v4/src/rag.c`](https://github.com/sneharao1811/PBL/blob/main/deadlock_v4/src/rag.c), generating bipartite graph topologies, assignment/request edges, ASCII terminal views, and JSON telemetry export.
8. **Interactive Graphical User Interface (GUI):** Developed the full browser console in [`deadlock_v4/gui/`](https://github.com/sneharao1811/PBL/blob/main/deadlock_v4/gui/), featuring an HTML5 Canvas RAG renderer, dynamic live matrix editor, step-by-step Banker's algorithm stepper, and local Python server runner.
9. **Project Laboratory Documentation:** Authored the comprehensive system documentation and technical report in [`README.md`](https://github.com/sneharao1811/PBL/blob/main/README.md).
10. **Message Queue IPC Research:** Investigated POSIX and System V message queue primitives in [`research/sneha_message_queues/sender.c`](https://github.com/sneharao1811/PBL/blob/main/research/sneha_message_queues/sender.c) and [`receiver.c`](https://github.com/sneharao1811/PBL/blob/main/research/sneha_message_queues/receiver.c), authoring the technical report `IPC Through Message Passing and Message Queues.pdf`.

---

## 2. Source Code Inventory

| File Path | Description |
| :--- | :--- |
| [`src_v2/ui/ui_main.c`](https://github.com/sneharao1811/PBL/blob/main/src_v2/ui/ui_main.c) | Interactive CLI shell: prompt display, command parser (`PUSH`, `POP`, `ADD`, `STATUS`, `EXIT`), IPC packet serialization, blocking response listener, and clean exit handler. |
| [`scheduler_v3/src/scheduler.c`](https://github.com/sneharao1811/PBL/blob/main/scheduler_v3/src/scheduler.c) *(Lines 44–98)* | First-Come-First-Served (`run_fcfs`) scheduling algorithm implementation. |
| [`scheduler_v3/src/scheduler.c`](https://github.com/sneharao1811/PBL/blob/main/scheduler_v3/src/scheduler.c) *(Lines 324–412)* | Round Robin (`run_round_robin`) time-sliced scheduling algorithm implementation. |
| [`deadlock_v4/src/deadlock_cli.c`](https://github.com/sneharao1811/PBL/blob/main/deadlock_v4/src/deadlock_cli.c) | Full 12-option interactive CLI shell: process/resource creation, matrix input, request dispatching, Banker's evaluation, detection, recovery, and status display. |
| [`deadlock_v4/src/rag.c`](https://github.com/sneharao1811/PBL/blob/main/deadlock_v4/src/rag.c) | Resource Allocation Graph engine: bipartite graph builder, assignment/request edge enumeration, ASCII terminal display, JSON telemetry export, and Graphviz DOT export. |
| [`deadlock_v4/gui/index.html`](https://github.com/sneharao1811/PBL/blob/main/deadlock_v4/gui/index.html) | GUI dashboard layout: header with presets bar, Canvas RAG panel, matrix editor panel, algorithm controls, request dispatch, and activity log stream. |
| [`deadlock_v4/gui/style.css`](https://github.com/sneharao1811/PBL/blob/main/deadlock_v4/gui/style.css) | Design system: dark systems-console theme with CSS custom properties, glassmorphism panels, pulse animations, responsive grid layout, and state-colored indicators. |
| [`deadlock_v4/gui/app.js`](https://github.com/sneharao1811/PBL/blob/main/deadlock_v4/gui/app.js) | GUI application controller (1174 lines): reactive state model, Canvas RAG renderer with animated edges, Banker's step-by-step playback stepper, detection/recovery engines, live matrix binding, resource request dispatcher, and event system. |
| [`README.md`](https://github.com/sneharao1811/PBL/blob/main/README.md) | Laboratory technical report, team matrix, sequence diagrams, benchmark comparisons, and user guide. |
| [`research/sneha_message_queues/sender.c`](https://github.com/sneharao1811/PBL/blob/main/research/sneha_message_queues/sender.c) | Phase 1 prototype: Message queue sender process demonstrating discrete message generation. |
| [`research/sneha_message_queues/receiver.c`](https://github.com/sneharao1811/PBL/blob/main/research/sneha_message_queues/receiver.c) | Phase 1 prototype: Message queue receiver process reading formatted IPC payloads. |
| [`research/sneha_message_queues/IPC Through Message Passing and Message Queues.pdf`](https://github.com/sneharao1811/PBL/tree/main/research/sneha_message_queues) | Technical research report on message queue APIs, kernel buffer thresholds, and asynchronous delivery semantics. |

---

## 3. Engineering Decisions & Architecture Rationale

### Dual Terminal & Stream Pipeline Execution
In [`src_v2/ui/ui_main.c`](https://github.com/sneharao1811/PBL/blob/main/src_v2/ui/ui_main.c), standard human interaction requires clear visual signposting (`SIM> `, banner text, status strings). However, running performance benchmarks via script redirection (`./bin/simulator_v2 < workload.txt`) would cause output buffer pollution if interactive prompts were printed on each line:
```c
int is_interactive = isatty(fileno(stdin));
if (is_interactive) {
    printf("\nSIM> ");
    fflush(stdout);
}
```
Using `isatty()`, the UI dynamically detects the execution context and avoids printing interactive prompts when input arrives via a pipeline or redirected file.

### Unidirectional Channel Separation
Instead of using a single bidirectional message queue, the architecture enforces two distinct POSIX queues:
1. `QUEUE_UI_TO_CORE` (`/pbl8_ui_to_core`): UI opens write-only (`O_WRONLY`), Core opens read-only (`O_RDONLY`).
2. `QUEUE_CORE_TO_UI` (`/pbl8_core_to_ui`): Core opens write-only (`O_WRONLY`), UI opens read-only (`O_RDONLY`).
This separation eliminates the risk of a process reading its own outgoing messages and guarantees deterministic request-reply RPC semantics.

### Time Quantum Trade-Offs: FCFS vs Round Robin
In [`scheduler_v3/src/scheduler.c`](https://github.com/sneharao1811/PBL/blob/main/scheduler_v3/src/scheduler.c), the implementation proved classical OS scheduling trade-offs:
- **FCFS** produces lower scheduling overhead because it is strictly non-preemptive. However, long jobs create a convoy effect for subsequent short tasks.
- **Round Robin ($q=3$)** preempts jobs after a fixed time slice, resulting in superior Average Response Time (**3.00 ms** vs FCFS's **8.75 ms** in Test Case 1), which is why time-sharing operating systems prefer Round Robin for interactive responsiveness.

---

## 4. Git Commit History

```text
30ad198 | 2026-10-22 | feat(deadlock-gui): implement interactive graphical user interface and visual RAG canvas
2fe2bc7 | 2026-10-21 | feat(deadlock): implement interactive CLI menu and RAG topological graph
0e617cf | 2026-10-13 | docs: comprehensive laboratory report and README update
9cc42f5 | 2026-10-11 | feat(scheduler): implement FCFS and Round Robin scheduling algorithms
b869442 | 2026-10-07 | feat(ui): implement interactive CLI frontend process
85634dd | 2026-09-30 | research: add message queues and IPC report
7043906 | 2026-09-16 | Initial commit
```
