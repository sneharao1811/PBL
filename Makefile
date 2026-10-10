.PHONY: all v1 v2 scheduler test clean benchmark

all: v1 v2 scheduler

v1:
	@echo "==> Building Baseline v1 (Monolithic Simulator)..."
	@$(MAKE) -C baseline_v1

v2:
	@echo "==> Building Architecture v2 (Multi-Process POSIX Simulator)..."
	@$(MAKE) -C src_v2

scheduler:
	@echo "==> Building Scheduler v3 (CPU Process Scheduling Simulator)..."
	@$(MAKE) -C scheduler_v3

test: all
	@echo "==> Running verification test on v2..."
	@printf "PUSH 42\nPUSH 58\nADD\nSTATUS\nEXIT\n" | ./bin/simulator_v2

benchmark: all
	@echo "==> Running Benchmark Suite..."
	@bash benchmarks/run_benchmark.sh

clean:
	@$(MAKE) -C baseline_v1 clean
	@$(MAKE) -C src_v2 clean
	@$(MAKE) -C scheduler_v3 clean
	@rm -rf bin simulator.log
