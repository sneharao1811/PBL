#!/usr/bin/env python3
import sys
import random

def generate_workload(count, filename):
    with open(filename, "w") as f:
        stack_depth = 0
        for _ in range(count):
            r = random.random()
            if stack_depth < 2 or r < 0.6:
                # Push
                val = random.randint(1, 1000)
                f.write(f"PUSH {val}\n")
                stack_depth += 1
            elif r < 0.85:
                # Add
                f.write("ADD\n")
                stack_depth -= 1
            elif r < 0.95:
                # Pop
                f.write("POP\n")
                stack_depth -= 1
            else:
                # Status
                f.write("STATUS\n")
        f.write("EXIT\n")
    print(f"Generated {count} instructions in {filename}")

if __name__ == "__main__":
    count = int(sys.argv[1]) if len(sys.argv) > 1 else 5000
    out = sys.argv[2] if len(sys.argv) > 2 else "benchmarks/workloads/workload_5000.txt"
    generate_workload(count, out)
