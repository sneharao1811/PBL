# Deadlock Simulation System — Architecture Specification

## 1. System Overview

The **Interactive Deadlock Simulation System (v4)** provides a comprehensive platform for demonstrating how an Operating System manages resource allocation, deadlock avoidance, detection, and recovery across multiple competing processes. The system implements three execution surfaces: a native C engine with 12-item interactive CLI, a cross-platform Python standalone runner, and a modern browser-based Graphical User Interface (GUI) with real-time Resource Allocation Graph visualization.

```mermaid
graph TD
    subgraph NativeEngine ["Native C Engine"]
        Header["deadlock.h<br/>(State Models & Contracts)"]
        Core["deadlock_core.c<br/>(Matrix Math & Presets)"]
        Banker["banker.c<br/>(Avoidance Engine)"]
        Detect["detection.c<br/>(Detection Algorithm)"]
        Recover["recovery.c<br/>(Recovery Strategies)"]
        RAG["rag.c<br/>(Graph Builder & Export)"]
        CLI["deadlock_cli.c<br/>(12-Item Interactive Shell)"]
    end

    subgraph PythonRunner ["Python Standalone Runner"]
        PySim["deadlock_sim.py<br/>(Complete Reimplementation)"]
    end

    subgraph BrowserGUI ["Interactive GUI"]
        HTML["index.html<br/>(Dashboard Layout)"]
        CSS["style.css<br/>(Design System)"]
        JS["app.js<br/>(Canvas RAG & Controllers)"]
        Server["server.py<br/>(HTTP Launcher)"]
    end

    Header --> Core & Banker & Detect & Recover & RAG
    Core --> CLI
    Banker --> CLI
    Detect --> CLI & Recover
    RAG --> CLI

    PySim -.-|"Algorithm Parity"| NativeEngine

    Server -->|"Serves"| HTML
    HTML --> CSS & JS
```

---

## 2. Mathematical Models & Data Structures

The system manages $N$ processes competing for $M$ resource types. All state is encapsulated in `deadlock_sys_t`:

```mermaid
classDiagram
    class deadlock_sys_t {
        +int num_processes
        +int num_resources
        +char process_names[MAX_PROCESSES][MAX_NAME_LEN]
        +char resource_names[MAX_RESOURCES][MAX_NAME_LEN]
        +int total_resources[MAX_RESOURCES]
        +int available[MAX_RESOURCES]
        +int allocation[MAX_PROCESSES][MAX_RESOURCES]
        +int maximum[MAX_PROCESSES][MAX_RESOURCES]
        +int need[MAX_PROCESSES][MAX_RESOURCES]
        +int request[MAX_PROCESSES][MAX_RESOURCES]
        +bool process_active[MAX_PROCESSES]
        +int process_priority[MAX_PROCESSES]
    }

    class safety_result_t {
        +bool is_safe
        +int safe_sequence[MAX_PROCESSES]
        +int sequence_len
        +char explanation[512]
    }

    class detection_result_t {
        +bool deadlock_detected
        +bool is_deadlocked[MAX_PROCESSES]
        +int deadlocked_processes[MAX_PROCESSES]
        +int deadlocked_count
        +char explanation[512]
    }

    class rag_graph_t {
        +int edge_count
        +rag_edge_t edges[]
        +bool has_cycle
        +int cycle_nodes[]
        +int cycle_len
    }

    deadlock_sys_t --> safety_result_t : bankers_check_safety()
    deadlock_sys_t --> detection_result_t : deadlock_detect()
    deadlock_sys_t --> rag_graph_t : rag_build_graph()
```

### Matrix Relationships

| Symbol | Name | Dimensions | Computation |
|:---:|:---|:---:|:---|
| $E$ | Total Resource Vector | $1 \times M$ | Static capacity per resource type |
| $A$ | Available Vector | $1 \times M$ | $A_j = E_j - \sum_{i=0}^{N-1} C_{i,j}$ |
| $C$ | Allocation Matrix | $N \times M$ | Resources currently held |
| $M$ | Maximum Claim Matrix | $N \times M$ | Declared peak demand |
| $N$ | Need Matrix | $N \times M$ | $N_{i,j} = M_{i,j} - C_{i,j}$ |
| $Q$ | Request Matrix | $N \times M$ | Currently pending/blocked requests |

---

## 3. Algorithmic Engines

### 3.1 Banker's Algorithm — Deadlock Avoidance (`banker.c`)

```mermaid
flowchart TD
    Start["bankers_check_safety()"] --> Init["Work = Available<br/>Finish[i] = false (for active)"]
    Init --> Search["Find process i where:<br/>Finish[i] == false AND<br/>Need[i] ≤ Work"]
    Search -->|"Found"| Simulate["Work += Allocation[i]<br/>Finish[i] = true<br/>Append i to Safe Sequence"]
    Simulate --> Search
    Search -->|"Not Found"| Check{"All Finish[i]<br/>== true?"}
    Check -->|"Yes"| Safe["✅ SAFE<br/>Return Safe Sequence"]
    Check -->|"No"| Unsafe["⚠️ UNSAFE<br/>List unfulfillable processes"]
```

### 3.2 Resource-Request Algorithm (`bankers_request_resource`)

```mermaid
flowchart TD
    Req["Process Pi requests vector R"] --> C1{"R ≤ Need[i]?"}
    C1 -->|"No"| Deny1["DENIED:<br/>Exceeds declared maximum"]
    C1 -->|"Yes"| C2{"R ≤ Available?"}
    C2 -->|"No"| Block["BLOCKED:<br/>Process must wait<br/>(request queued)"]
    C2 -->|"Yes"| Hypo["Hypothetical allocation:<br/>Available -= R<br/>Allocation[i] += R<br/>Need[i] -= R"]
    Hypo --> Safety{"Run Safety Check<br/>on hypothetical state"}
    Safety -->|"Safe"| Grant["✅ GRANTED<br/>Commit allocation"]
    Safety -->|"Unsafe"| Reject["⛔ DENIED BY AVOIDANCE<br/>Rollback, queue request"]
```

### 3.3 Multi-Instance Deadlock Detection (`detection.c`)

```mermaid
flowchart TD
    Start["deadlock_detect()"] --> Init["Work = Available<br/>Finish[i] = true if Allocation[i] == 0"]
    Init --> Scan["Find process i where:<br/>Finish[i] == false AND<br/>Request[i] ≤ Work"]
    Scan -->|"Found"| Release["Work += Allocation[i]<br/>Finish[i] = true"]
    Release --> Scan
    Scan -->|"Not Found"| Eval{"Any Finish[i]<br/>== false<br/>(active process)?"}
    Eval -->|"Yes"| DL["🔴 DEADLOCK<br/>List all blocked PIDs"]
    Eval -->|"No"| NoDL["🟢 NO DEADLOCK"]
```

### 3.4 Recovery Engine (`recovery.c`)

```mermaid
flowchart TD
    Trigger["Deadlock Confirmed"] --> Strategy{"Recovery<br/>Strategy?"}
    Strategy -->|"1: Terminate All"| AbortAll["Abort all deadlocked<br/>Reclaim all allocations"]
    Strategy -->|"2: Selective Victim"| Loop["Select victim with<br/>max held resources"]
    Loop --> Kill["Terminate victim<br/>Reclaim resources"]
    Kill --> ReCheck{"Still<br/>deadlocked?"}
    ReCheck -->|"Yes"| Loop
    ReCheck -->|"No"| Done["✅ Cycle broken"]
    Strategy -->|"3: Preemption"| Preempt["Preempt victim's<br/>allocations<br/>Reset Need, rollback"]
    AbortAll --> Done
    Preempt --> Done
```

---

## 4. GUI Architecture

The GUI is a zero-dependency single-page application served by a local Python HTTP server.

```mermaid
graph LR
    subgraph Server ["server.py (Python HTTP)"]
        HTTPHandler["SimpleHTTPRequestHandler"]
        AutoOpen["webbrowser.open()"]
    end

    subgraph Frontend ["Browser Application"]
        subgraph Layout ["index.html"]
            Header["Header & Presets Bar"]
            RAGPanel["Canvas RAG Panel"]
            MatrixPanel["Matrix Editor Panel"]
            AlgoPanel["Algorithm Panel"]
            RequestPanel["Request Dispatch Panel"]
            LogPanel["Activity Log"]
        end

        subgraph Styles ["style.css"]
            DesignTokens["Color Tokens & Typography"]
            Animations["Pulse, Glow, Transitions"]
            Responsive["Grid Layout System"]
        end

        subgraph Logic ["app.js (1174 lines)"]
            StateModel["Reactive Data Model"]
            Presets["Preset Initializers"]
            MatrixCalc["Matrix Recalculation"]
            BankerJS["Banker's Safety Algorithm"]
            DetectionJS["Detection Algorithm"]
            RecoveryJS["Recovery Engine"]
            CanvasRAG["Canvas RAG Renderer"]
            Stepper["Step-by-Step Playback"]
            EventBindings["UI Event Handlers"]
        end
    end

    Server -->|"HTTP GET"| Frontend
    StateModel --> MatrixCalc --> BankerJS & DetectionJS
    DetectionJS --> RecoveryJS
    StateModel --> CanvasRAG
    BankerJS --> Stepper
    EventBindings --> StateModel
```

### GUI Feature Matrix

| Feature | Implementation |
|:---|:---|
| **Interactive Canvas RAG** | HTML5 Canvas with animated process circles, resource rectangles, assignment edges (green), request edges (amber), deadlock highlighting (crimson pulse) |
| **Live Matrix Editor** | Editable allocation/maximum tables with real-time need recalculation |
| **Banker's Step-by-Step Playback** | Animated stepper showing Work vector progression and Finish array updates per iteration |
| **Resource Request Dispatcher** | Process selector + resource vector inputs with Banker's avoidance feedback |
| **Resource Release** | One-click release returning allocations to Available pool |
| **3 Recovery Strategies** | Terminate All, Selective Victim, Resource Preemption — all with visual state updates |
| **System State Badge** | Live SAFE (green) / UNSAFE (amber) / DEADLOCKED (red) indicator with pulse animation |
| **Resource Utilization Meters** | Per-resource percentage bars showing allocated vs. free capacity |
| **Activity Log Stream** | Timestamped event log with severity coloring (success/warning/error/info) |
| **3 Preset Scenarios** | Textbook Safe (Silberschatz 5P/3R), Circular Deadlock (3P mutual wait), Unsafe Demand (4P/2R) |

---

## 5. Cross-Platform Execution Surfaces

| Surface | Technology | Entry Point | Platform |
|:---|:---|:---|:---|
| **Native CLI** | C11 / GCC | `./bin/deadlock_cli` or `make deadlock` | Linux, WSL2 |
| **Python CLI** | Python 3.8+ | `python deadlock_v4/deadlock_sim.py` | All platforms |
| **Browser GUI** | HTML5 / JS / CSS | `python deadlock_v4/gui/server.py` or `make deadlock-gui` | All platforms |
| **Windows Shortcut** | Batch Script | `bin/deadlock_gui.bat` | Windows |

---

## 6. Module Ownership & Contribution Map

| Module | File(s) | Primary Author | Contribution |
|:---|:---|:---:|:---|
| Data Models & State Engine | [`deadlock_v4/include/deadlock.h`](https://github.com/sneharao1811/PBL/blob/main/deadlock_v4/include/deadlock.h), [`deadlock_v4/src/deadlock_core.c`](https://github.com/sneharao1811/PBL/blob/main/deadlock_v4/src/deadlock_core.c) | Waqqas | Struct definitions, matrix math, presets, telemetry formatters, headroom validation |
| Banker's Avoidance Engine | [`deadlock_v4/src/banker.c`](https://github.com/sneharao1811/PBL/blob/main/deadlock_v4/src/banker.c) | Siddanth | Safety algorithm, resource-request validator, resource release handler |
| Detection Algorithm | [`deadlock_v4/src/detection.c`](https://github.com/sneharao1811/PBL/blob/main/deadlock_v4/src/detection.c) | Rafath | Multi-instance request-matrix reduction, deadlocked PID enumeration |
| Recovery Engine | [`deadlock_v4/src/recovery.c`](https://github.com/sneharao1811/PBL/blob/main/deadlock_v4/src/recovery.c) | Rafath | Victim selection heuristic, 3 recovery strategies, resource reclamation |
| 12-Item CLI Shell | [`deadlock_v4/src/deadlock_cli.c`](https://github.com/sneharao1811/PBL/blob/main/deadlock_v4/src/deadlock_cli.c) | Sneha | Full interactive menu loop, input parsing, formatted output |
| RAG Visualizer | [`deadlock_v4/src/rag.c`](https://github.com/sneharao1811/PBL/blob/main/deadlock_v4/src/rag.c) | Sneha | Graph builder, ASCII/JSON/DOT export, cycle detection delegation |
| Interactive GUI | [`deadlock_v4/gui/`](https://github.com/sneharao1811/PBL/tree/main/deadlock_v4/gui/) | Sneha | Canvas rendering, live matrix binding, stepper, recovery controls |
| Python Runner | [`deadlock_v4/deadlock_sim.py`](https://github.com/sneharao1811/PBL/blob/main/deadlock_v4/deadlock_sim.py) | Waqqas | Algorithm-parity standalone for cross-platform execution |
| Build System | [`Makefile`](https://github.com/sneharao1811/PBL/blob/main/Makefile), [`deadlock_v4/Makefile`](https://github.com/sneharao1811/PBL/blob/main/deadlock_v4/Makefile) | Waqqas | Compilation targets, GUI launcher, test harness |
| HTTP Server | [`deadlock_v4/gui/server.py`](https://github.com/sneharao1811/PBL/blob/main/deadlock_v4/gui/server.py) | Waqqas | Zero-dependency local server with auto-browser launch |

---

## 7. Build & Execution

```bash
# Build native C CLI (Linux/WSL2)
make deadlock

# Run native 12-item CLI
./bin/deadlock_cli

# Run cross-platform Python CLI
python deadlock_v4/deadlock_sim.py

# Launch interactive GUI (opens browser automatically)
make deadlock-gui
# or
python deadlock_v4/gui/server.py

# Run automated verification test
make test-deadlock
```
