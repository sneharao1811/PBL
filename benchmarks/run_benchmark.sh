#!/usr/bin/env bash
set -e

WORKLOAD_COUNT=1000
WORKLOAD_FILE="benchmarks/workloads/workload_${WORKLOAD_COUNT}.txt"
RESULTS_FILE="benchmarks/results.txt"

echo "=========================================================="
echo "      PBL Team 8: Benchmark & Performance Suite           "
echo "  Comparing Single-Process Baseline vs Multi-Process IPC  "
echo "=========================================================="

# Ensure binaries are built
make v1 > /dev/null
make v2 > /dev/null

# Generate workload if missing
if [ ! -f "$WORKLOAD_FILE" ]; then
    echo "[*] Generating synthetic workload ($WORKLOAD_COUNT instructions)..."
    python3 benchmarks/generate_workload.py $WORKLOAD_COUNT "$WORKLOAD_FILE"
fi

echo "[*] Benchmarking Baseline v1 (Single-Process Monolith)..."
rm -f simulator.log
START_V1=$(python3 -c 'import time; print(int(time.time() * 1000000))')
./baseline_v1/simulator_v1 < "$WORKLOAD_FILE" > /dev/null 2>&1 || true
END_V1=$(python3 -c 'import time; print(int(time.time() * 1000000))')
TIME_V1_US=$((END_V1 - START_V1))
TIME_V1_MS=$(python3 -c "print(f'{$TIME_V1_US / 1000:.2f}')")

echo "[*] Benchmarking Architecture v2 (3-Process POSIX Queue IPC)..."
rm -f simulator.log
START_V2=$(python3 -c 'import time; print(int(time.time() * 1000000))')
./bin/simulator_v2 < "$WORKLOAD_FILE" > /dev/null 2>&1 || true
END_V2=$(python3 -c 'import time; print(int(time.time() * 1000000))')
TIME_V2_US=$((END_V2 - START_V2))
TIME_V2_MS=$(python3 -c "print(f'{$TIME_V2_US / 1000:.2f}')")

OVERHEAD_US=$((TIME_V2_US - TIME_V1_US))
OVERHEAD_MS=$(python3 -c "print(f'{$OVERHEAD_US / 1000:.2f}')")
PER_OP_OVERHEAD_US=$(python3 -c "print(f'{$OVERHEAD_US / $WORKLOAD_COUNT:.2f}')")
THROUGHPUT_V1=$(python3 -c "print(f'{$WORKLOAD_COUNT / ($TIME_V1_US / 1000000):.0f}')")
THROUGHPUT_V2=$(python3 -c "print(f'{$WORKLOAD_COUNT / ($TIME_V2_US / 1000000):.0f}')")

REPORT="
================================================================================
                    BENCHMARK PERFORMANCE COMPARISON
================================================================================
Workload Size               : $WORKLOAD_COUNT instructions (PUSH, POP, ADD, STATUS)
--------------------------------------------------------------------------------
Metric                      | Single-Process (v1)   | Multi-Process IPC (v2)
--------------------------------------------------------------------------------
Total Wall Time             | ${TIME_V1_MS} ms               | ${TIME_V2_MS} ms
Throughput                  | ${THROUGHPUT_V1} ops/sec           | ${THROUGHPUT_V2} ops/sec
Mean Latency per Op         | $(python3 -c "print(f'{$TIME_V1_US / $WORKLOAD_COUNT:.2f}')") us               | $(python3 -c "print(f'{$TIME_V2_US / $WORKLOAD_COUNT:.2f}')") us
Memory & Fault Isolation    | None (Shared space)   | Strong (3 separated PIDs)
Logging Model               | Synchronous File I/O  | Asynchronous Daemon + Thread
--------------------------------------------------------------------------------
IPC Overhead Analysis:
* Total Overhead Added      : ${OVERHEAD_MS} ms
* Average IPC Cost per Msg  : ${PER_OP_OVERHEAD_US} microseconds
================================================================================
"

echo "$REPORT"
echo "$REPORT" > "$RESULTS_FILE"
echo "[+] Saved benchmark results to $RESULTS_FILE"
