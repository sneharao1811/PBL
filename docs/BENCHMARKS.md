# Performance Benchmarking & Analysis Report

## 1. Benchmark Methodology

The benchmark suite (`benchmarks/run_benchmark.sh`) evaluates the performance of the **Monolithic Baseline (`v1`)** versus the **Multi-Process Architecture (`v2`)** under identical synthetic workloads.

* **Workload Size**: 1,000 synthetic operations (`PUSH`, `POP`, `ADD`, `STATUS`, `EXIT`).
* **Hardware/OS**: Ubuntu 22.04 LTS via WSL2 on AMD/Intel x86_64 CPU.
* **Measurement**: Wall-clock execution time calculated via high-precision epoch timestamps (`time.time()`).

---

## 2. Empirical Benchmark Results

```text
================================================================================
                    BENCHMARK PERFORMANCE COMPARISON
================================================================================
Workload Size               : 1000 instructions (PUSH, POP, ADD, STATUS)
--------------------------------------------------------------------------------
Metric                      | Single-Process (v1)   | Multi-Process IPC (v2)
--------------------------------------------------------------------------------
Total Wall Time             | 2442.27 ms            | 339.82 ms
Throughput                  | 409 ops/sec           | 2943 ops/sec
Mean Latency per Op         | 2442.27 us            | 339.82 us
Memory & Fault Isolation    | None (Shared space)   | Strong (3 separated PIDs)
Logging Model               | Synchronous File I/O  | Asynchronous Daemon + Thread
--------------------------------------------------------------------------------
Key Takeaways:
* Multi-Process Speedup     : ~7.18x Higher Throughput
* IPC Overhead Mitigation   : Multithreaded ring-buffer completely masks I/O cost
================================================================================
```

---

## 3. Analysis & Key Observations

1. **The I/O Bottleneck in Monolithic Systems**:
   In `v1`, each CPU instruction synchronously opens, writes, and closes `simulator.log` on disk (`fopen("simulator.log", "a")` + `fclose()`). This causes the execution core to stall on every instruction waiting for disk buffer flushes.
   
2. **How Multi-Process Decomposition Won**:
   In `v2`, the Core CPU engine executes in RAM and enqueues log messages into an internal memory ring buffer in microseconds. An asynchronous background thread transmits these to the dedicated Logger Daemon via POSIX Message Queues (`/pbl8_to_logger`). The Logger Daemon performs line-buffered disk writes in parallel with ongoing CPU operations.

3. **Fault Isolation**:
   If the UI or Logger crashes, the Core execution state (Stack, Memory, ALU) remains intact in its isolated memory space (`PID`), preventing memory corruption across process boundaries.
