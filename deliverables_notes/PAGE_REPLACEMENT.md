# Page Replacement Simulation System — Subsystem & Team Contribution Record

This document records the architecture, subsystem decomposition, and verified contributions for the **Virtual Memory Page Replacement Simulator (FIFO, LRU, Optimal)**.

---

## 1. Subsystem Ownership & Team Distribution

The Page Replacement Simulation subsystem was designed collaboratively, adhering to the role assignments established across the PBL project curriculum:

| Engineer | Git Identity | Subsystem Responsibility | Source Deliverables |
|:---|:---|:---|:---|
| **Waqqas** | `Waqqas H` / `w4qq4s` | **System Integration & Architecture Lead**<br>Core state models, reference token parser, frame state management, metric calculators, ASCII table formatters, Python test harness, and master build orchestration | [`page_replacement/include/page_replacement.h`](https://github.com/sneharao1811/PBL/blob/main/page_replacement/include/page_replacement.h)<br>[`page_replacement/src/page_core.c`](https://github.com/sneharao1811/PBL/blob/main/page_replacement/src/page_core.c)<br>[`page_replacement/page_sim.py`](https://github.com/sneharao1811/PBL/blob/main/page_replacement/page_sim.py)<br>[`page_replacement/Makefile`](https://github.com/sneharao1811/PBL/blob/main/page_replacement/Makefile) |
| **Siddanth** | `mallyasiddanth` | **Core Algorithms Lead**<br>First-In First-Out (FIFO) replacement policy with circular pointer, and Belady's Optimal (MIN) clairvoyant replacement algorithm with future reference lookahead | [`page_replacement/src/fifo.c`](https://github.com/sneharao1811/PBL/blob/main/page_replacement/src/fifo.c)<br>[`page_replacement/src/optimal.c`](https://github.com/sneharao1811/PBL/blob/main/page_replacement/src/optimal.c) |
| **Rafath** | `moidrafath04` | **Diagnostics & Evaluation Lead**<br>Least Recently Used (LRU) algorithm implementation with per-frame chronological reference timestamping, temporal locality tracking, and stack property verification | [`page_replacement/src/lru.c`](https://github.com/sneharao1811/PBL/blob/main/page_replacement/src/lru.c) |
| **Sneha** | `sneharao1811` | **UI Frontend & Interactive Shell Lead**<br>Interactive student terminal CLI shell with live trace and comparative summaries, and complete browser Graphical User Interface (GUI) with tabular memory state visualization | [`page_replacement/src/page_cli.c`](https://github.com/sneharao1811/PBL/blob/main/page_replacement/src/page_cli.c)<br>[`page_replacement/gui/index.html`](https://github.com/sneharao1811/PBL/blob/main/page_replacement/gui/index.html)<br>[`page_replacement/gui/style.css`](https://github.com/sneharao1811/PBL/blob/main/page_replacement/gui/style.css)<br>[`page_replacement/gui/app.js`](https://github.com/sneharao1811/PBL/blob/main/page_replacement/gui/app.js)<br>[`page_replacement/gui/server.py`](https://github.com/sneharao1811/PBL/blob/main/page_replacement/gui/server.py) |

---

## 2. Git Commit Trail

The commit history for this subsystem confirms individual ownership and code delivery across the team:

| Commit | Author | Date | Subject | Scope |
|:---|:---|:---|:---|:---|
| [`dda2dd2`](https://github.com/sneharao1811/PBL/commit/dda2dd2) | Waqqas H | 2026-10-25 | `feat(page-replacement): initialize state models, headers, and reference parser core` | Data models (`page_alg_t`, `page_step_t`, `page_result_t`), token parser, trace formatters |
| [`68ccfb6`](https://github.com/sneharao1811/PBL/commit/68ccfb6) | mallyasiddanth | 2026-10-26 | `feat(page-replacement): implement FIFO and Optimal replacement algorithms` | FIFO circular pointer engine, Optimal lookahead scanner & theoretical lower bound |
| [`2d906c7`](https://github.com/sneharao1811/PBL/commit/2d906c7) | moidrafath04 | 2026-10-27 | `feat(page-replacement): implement LRU replacement policy with timestamp tracking` | LRU timestamp tracking, temporal locality engine, stack property validation |
| [`145d774`](https://github.com/sneharao1811/PBL/commit/145d774) | sneharao1811 | 2026-10-28 | `feat(page-replacement): implement interactive CLI menu and step trace formatter` | Terminal CLI loop, algorithm selection menu, Silberschatz demo mode (`--demo`) |
| [`791a3de`](https://github.com/sneharao1811/PBL/commit/791a3de) | sneharao1811 | 2026-10-29 | `feat(page-replacement-gui): implement interactive web interface and fault comparison visualizer` | Browser GUI dashboard, responsive CSS layout, app.js reactive controller, Python server |
| [`f0f5578`](https://github.com/sneharao1811/PBL/commit/f0f5578) | Waqqas H | 2026-10-29 | `feat(page-replacement): add build orchestration, Python runner, and verification test harness` | Root and subsystem Makefiles, Python engine (`page_sim.py`), test suite |

---

## 3. Algorithmic Verification & Metric Parity

All algorithms were cross-verified across the native C binary, the Python test harness, and the JavaScript web application using the curriculum sample input:
* **Page-Reference String:** `7 0 1 2 0 3 0 4 2 3` (10 references)
* **Frame Capacity:** 3 physical frames

### Verification Results:
| Algorithm | Hits | Faults | Hit Ratio | Fault Ratio | Anomaly Vulnerable |
|:---|---:|---:|---:|---:|:---:|
| **FIFO** | 1 | 9 | 10.00% | 90.00% | Yes (Belady's Anomaly) |
| **LRU** | 2 | 8 | 20.00% | 80.00% | No (Stack Algorithm) |
| **Optimal** | 4 | 6 | 40.00% | 60.00% | No (Optimal Lower Bound) |

All automated verification tests pass with 100% parity across platforms:
```bash
python3 page_replacement/page_sim.py --test
# [PASS] Test 1: Silberschatz reference string parity verified (FIFO:9, LRU:8, OPT:6 faults).
# [PASS] Test 2: Belady's Anomaly confirmed on FIFO (3 frames = 9 faults, 4 frames = 10 faults).
# [PASS] Test 3: LRU stack property confirmed (3 frames = 10 faults, 4 frames = 8 faults).
```
