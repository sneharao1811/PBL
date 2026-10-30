# Virtual Memory Page Replacement Simulation Architecture

## 1. System Overview & Problem Statement

The **Virtual Memory Page Replacement Simulator** models physical memory frame allocation and evaluates demand-paging replacement policies in an operating system. When a virtual memory system encounters a page reference for a page not currently resident in physical frames, a **page fault** occurs. If physical memory is completely occupied, the OS page replacement engine must evict an existing resident page to accommodate the incoming page.

This simulator models three fundamental page replacement policies specified in the OS curriculum:
1. **First-In, First-Out (FIFO)**
2. **Least Recently Used (LRU)**
3. **Belady's Optimal Replacement (OPT / MIN)**

The system delivers dual operational surfaces:
- **High-Performance Native C CLI (`bin/page_replacement_cli`)**: Features interactive terminal input, per-reference memory frame evolution tables, hit/fault badges, performance summary statistics, and automated Silberschatz demonstration runs (`--demo`).
- **Interactive Browser GUI (`page_replacement/gui/`)**: Built with responsive vanilla HTML/CSS/JS with zero external runtime dependencies, providing live tabular frame snapshots, hit/fault counters, hit and fault ratios, and a comparative fault bar visualizer.
- **Cross-Platform Python Engine & Verification Harness (`page_sim.py`)**: Implements strict algorithmic parity, validation tests, and mathematical verification of Belady's Anomaly and LRU stack properties.

---

## 2. Mathematical Formalization & Metrics

For a given page reference string $\mathcal{R} = \langle r_1, r_2, \dots, r_m \rangle$ with $m$ references, and physical memory capacity $F$ frames ($f_0, f_1, \dots, f_{F-1}$):

At each discrete time step $t \in [1, m]$ with requested page $r_t$:
- **Resident State:** A page $r_t$ is **resident** (Page Hit) if $\exists j \in [0, F-1]$ such that $f_j = r_t$.
- **Page Fault:** If $r_t \notin \{f_0, \dots, f_{F-1}\}$, a page fault occurs:
  - If $\exists j$ such that $f_j = \text{EMPTY}$, page $r_t$ is placed into frame $j$.
  - If all frames are occupied, an eviction policy $\mathcal{P}$ selects a victim frame index $v \in [0, F-1]$, evicts resident page $f_v$, and assigns $f_v \leftarrow r_t$.

### Performance Evaluation Metrics:
$$\text{Total References} = m$$
$$\text{Total Page Hits} = H, \quad \text{Total Page Faults} = M$$
$$H + M = m$$
$$\text{Hit Ratio } (\mathcal{H}) = \frac{H}{m} \times 100\%$$
$$\text{Fault Ratio } (\mathcal{M}) = \frac{M}{m} \times 100\%$$
$$\mathcal{H} + \mathcal{M} = 100\%$$

---

## 3. Algorithmic Policies & Implementation Details

### A. First-In, First-Out (FIFO)
* **Mechanics:** Tracks the chronological order of frame allocations using a circular pointer $p \in [0, F-1]$. When a page fault occurs under full memory, the frame pointed to by $p$ is replaced, and the pointer advances: $p \leftarrow (p + 1) \pmod F$.
* **Hit Behavior:** A page hit does not alter the queue or replacement order.
* **Complexity:** Time $O(1)$ per reference; Space $O(F)$.
* **Vulnerability:** **Belady's Anomaly** — increasing the physical frame capacity can paradoxically increase total page faults for certain non-stack reference patterns.

### B. Least Recently Used (LRU)
* **Mechanics:** Evicts the page that has not been referenced for the longest period. Implemented via chronological timestamp tracking: array `last_used[F]` records the step index $t$ when frame $f_j$ was accessed.
* **Hit Behavior:** On a page hit for frame $j$, `last_used[j]` is refreshed to the current step $t$.
* **Victim Selection:** $v = \arg\min_{j} \text{last\_used}[j]$.
* **Complexity:** Time $O(F)$ per reference; Space $O(F)$.
* **Properties:** LRU is a **stack algorithm** where $M(F) \ge M(F+1)$ for all reference strings. It is mathematically immune to Belady's Anomaly.

### C. Belady's Optimal Replacement (OPT / MIN)
* **Mechanics:** Evicts the resident page whose next reference occurs farthest in the future (or is never referenced again).
* **Future Lookahead:** For each resident frame $j$, compute next reference step $n_j = \min \{ k > t \mid r_k = f_j \}$. If page $f_j$ does not appear in future references, $n_j = \infty$.
* **Victim Selection:** $v = \arg\max_{j} n_j$. Ties are resolved deterministically to the lowest-numbered frame index.
* **Complexity:** Time $O(F \times (m - t))$ per reference; Space $O(F)$.
* **Significance:** Optimal establishes the theoretical minimum fault lower bound for any offline reference string. It cannot be implemented in general-purpose online operating systems because future page accesses cannot be predicted in advance.

---

## 4. Subsystem Decomposition & Source Inventory

```
page_replacement/
├── include/
│   └── page_replacement.h          # Central headers, data structures, prototypes
├── src/
│   ├── page_core.c                 # Token parser, frame state manager, formatters
│   ├── fifo.c                      # FIFO circular pointer algorithm
│   ├── lru.c                       # LRU timestamp-based replacement engine
│   ├── optimal.c                   # Optimal clairvoyant future scanner
│   └── page_cli.c                  # Interactive student terminal CLI menu
├── gui/
│   ├── index.html                  # Responsive simulator dashboard
│   ├── style.css                   # System styling & visual hierarchy
│   ├── app.js                      # Browser simulation engine & chart renderer
│   └── server.py                   # Zero-dependency local Python HTTP launcher
├── page_sim.py                     # Standalone Python runner & test suite
└── Makefile                        # Native build automation
```

---

## 5. Verification Benchmark & Reference String Parity

The simulator was verified against the curriculum test dataset from Silberschatz:
* **Reference String:** `7 0 1 2 0 3 0 4 2 3` ($m = 10$)
* **Frame Count:** 3 frames

### Verification Results Matrix:
| Step | Page | FIFO (Frames: F1, F2, F3) | FIFO Status | LRU (Frames: F1, F2, F3) | LRU Status | Optimal (Frames: F1, F2, F3) | Optimal Status |
|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|
| 1 | 7 | [7, -, -] | Fault | [7, -, -] | Fault | [7, -, -] | Fault |
| 2 | 0 | [7, 0, -] | Fault | [7, 0, -] | Fault | [7, 0, -] | Fault |
| 3 | 1 | [7, 0, 1] | Fault | [7, 0, 1] | Fault | [7, 0, 1] | Fault |
| 4 | 2 | [2, 0, 1] | Fault | [2, 0, 1] | Fault | [2, 0, 1] | Fault |
| 5 | 0 | [2, 0, 1] | **Hit** | [2, 0, 1] | **Hit** | [2, 0, 1] | **Hit** |
| 6 | 3 | [2, 3, 1] | Fault | [2, 0, 3] | Fault | [2, 0, 3] | Fault |
| 7 | 0 | [2, 3, 0] | Fault | [2, 0, 3] | **Hit** | [2, 0, 3] | **Hit** |
| 8 | 4 | [4, 3, 0] | Fault | [4, 0, 3] | Fault | [2, 4, 3] | Fault |
| 9 | 2 | [4, 2, 0] | Fault | [4, 0, 2] | Fault | [2, 4, 3] | **Hit** |
| 10 | 3 | [4, 2, 3] | Fault | [4, 3, 2] | Fault | [2, 4, 3] | **Hit** |

### Summary Comparison:
| Algorithm | Hits | Faults | Hit Ratio | Fault Ratio | Relative Performance |
|:---|---:|---:|---:|---:|:---|
| **FIFO** | 1 | 9 | 10.00% | 90.00% | Baseline |
| **LRU** | 2 | 8 | 20.00% | 80.00% | +100% hits over FIFO |
| **Optimal** | 4 | 6 | 40.00% | 60.00% | **Theoretical Best (Lower Bound)** |

---

## 6. Execution & Quickstart Guide

### 1. Run Interactive Native C CLI
```bash
# Build the native binary
make page-replacement

# Run interactive session
./bin/page_replacement_cli

# Run quick automated demo
./bin/page_replacement_cli --demo
```

### 2. Launch Browser Graphical Interface
```bash
# Serves on http://127.0.0.1:8090 and opens browser automatically:
make page-replacement-gui

# Or run directly via Python:
python3 page_replacement/gui/server.py
```

### 3. Run Automated Python Test Suite
```bash
make test-page-replacement
# or
python3 page_replacement/page_sim.py --test
```
