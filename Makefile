.PHONY: all v1 v2 scheduler deadlock deadlock-gui page-replacement page-replacement-cli page-replacement-gui test test-deadlock test-page-replacement clean benchmark

all: v1 v2 scheduler deadlock page-replacement

v1:
	@echo "==> Building Baseline v1 (Monolithic Simulator)..."
	@$(MAKE) -C baseline_v1

v2:
	@echo "==> Building Architecture v2 (Multi-Process POSIX Simulator)..."
	@$(MAKE) -C src_v2

scheduler:
	@echo "==> Building Scheduler v3 (CPU Process Scheduling Simulator)..."
	@$(MAKE) -C scheduler_v3

deadlock:
	@echo "==> Building Deadlock Simulation System v4..."
	@$(MAKE) -C deadlock_v4

deadlock-gui:
	@echo "==> Launching Deadlock Simulation System GUI..."
	@python3 deadlock_v4/gui/server.py

page-replacement:
	@echo "==> Building Page Replacement Simulator..."
	@$(MAKE) -C page_replacement

page-replacement-cli: page-replacement
	@echo "==> Running Page Replacement Simulator CLI..."
	@./bin/page_replacement_cli

page-replacement-gui:
	@echo "==> Launching Page Replacement Simulator GUI..."
	@python3 page_replacement/gui/server.py

test: all
	@echo "==> Running verification test on v2..."
	@printf "PUSH 42\nPUSH 58\nADD\nSTATUS\nEXIT\n" | ./bin/simulator_v2

test-deadlock:
	@echo "==> Running verification test on Deadlock Simulation System..."
	@python3 -c "import subprocess; p = subprocess.Popen(['python3', 'deadlock_v4/deadlock_sim.py'], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True); out, err = p.communicate('8\n9\n11\n12\n'); print(out)"

test-page-replacement: page-replacement
	@echo "==> Running verification test on Page Replacement System..."
	@python3 page_replacement/page_sim.py --test

benchmark: all
	@echo "==> Running Benchmark Suite..."
	@bash benchmarks/run_benchmark.sh

clean:
	@$(MAKE) -C baseline_v1 clean
	@$(MAKE) -C src_v2 clean
	@$(MAKE) -C scheduler_v3 clean
	@$(MAKE) -C deadlock_v4 clean
	@$(MAKE) -C page_replacement clean
	@rm -f bin/*.o bin/deadlock_cli bin/deadlock_cli.exe bin/page_replacement_cli bin/page_replacement_cli.exe simulator.log
