#!/usr/bin/env python3
"""
Interactive Page Replacement Simulator & Test Harness
Evaluates FIFO, LRU, and Optimal Page Replacement Policies with step tracing,
hit/fault metrics, comparative evaluation, and Belady Anomaly verification.

Team 8 - Systems & Operating Systems PBL
Author: Waqqas H (Lead Systems Architect & Integration Lead)
"""

import sys
from typing import List, Dict, Any, Tuple


def parse_reference_string(raw: str) -> List[int]:
    """Parse space or comma delimited string into integer page references."""
    tokens = raw.replace(',', ' ').split()
    if not tokens:
        raise ValueError("Reference string cannot be empty.")
    pages = []
    for t in tokens:
        val = int(t)
        if val < 0:
            raise ValueError(f"Page numbers must be non-negative (got {val}).")
        pages.append(val)
    return pages


def simulate_fifo(pages: List[int], frame_count: int) -> Dict[str, Any]:
    """Simulate First-In First-Out (FIFO) replacement policy."""
    frames = [-1] * frame_count
    pointer = 0
    hits = 0
    faults = 0
    steps = []

    for idx, page in enumerate(pages):
        evicted = -1
        if page in frames:
            hits += 1
            is_hit = True
            resident_idx = frames.index(page)
            action = f"Page {page} resident in Frame {resident_idx + 1}"
        else:
            faults += 1
            is_hit = False
            if -1 in frames:
                empty_idx = frames.index(-1)
                frames[empty_idx] = page
                action = f"Loaded page {page} into Frame {empty_idx + 1}"
            else:
                evicted = frames[pointer]
                frames[pointer] = page
                action = f"Replaced page {evicted} in Frame {pointer + 1}"
                pointer = (pointer + 1) % frame_count

        steps.append({
            "step": idx + 1,
            "page": page,
            "frames": list(frames),
            "is_hit": is_hit,
            "evicted": evicted,
            "action": action
        })

    total = len(pages)
    return {
        "algorithm": "FIFO",
        "total": total,
        "frames_count": frame_count,
        "hits": hits,
        "faults": faults,
        "hit_ratio": (hits / total) * 100.0 if total else 0.0,
        "fault_ratio": (faults / total) * 100.0 if total else 0.0,
        "steps": steps
    }


def simulate_lru(pages: List[int], frame_count: int) -> Dict[str, Any]:
    """Simulate Least Recently Used (LRU) replacement policy."""
    frames = [-1] * frame_count
    last_used = [-1] * frame_count
    hits = 0
    faults = 0
    steps = []

    for idx, page in enumerate(pages):
        evicted = -1
        if page in frames:
            hits += 1
            is_hit = True
            f_idx = frames.index(page)
            last_used[f_idx] = idx
            action = f"Page {page} resident in Frame {f_idx + 1} (refreshed)"
        else:
            faults += 1
            is_hit = False
            if -1 in frames:
                empty_idx = frames.index(-1)
                frames[empty_idx] = page
                last_used[empty_idx] = idx
                action = f"Loaded page {page} into Frame {empty_idx + 1}"
            else:
                # Find frame with minimum last_used timestamp
                victim_idx = min(range(frame_count), key=lambda i: last_used[i])
                evicted = frames[victim_idx]
                frames[victim_idx] = page
                last_used[victim_idx] = idx
                action = f"Replaced page {evicted} in Frame {victim_idx + 1} (LRU)"

        steps.append({
            "step": idx + 1,
            "page": page,
            "frames": list(frames),
            "is_hit": is_hit,
            "evicted": evicted,
            "action": action
        })

    total = len(pages)
    return {
        "algorithm": "LRU",
        "total": total,
        "frames_count": frame_count,
        "hits": hits,
        "faults": faults,
        "hit_ratio": (hits / total) * 100.0 if total else 0.0,
        "fault_ratio": (faults / total) * 100.0 if total else 0.0,
        "steps": steps
    }


def simulate_optimal(pages: List[int], frame_count: int) -> Dict[str, Any]:
    """Simulate Belady's Optimal (OPT/MIN) clairvoyant replacement policy."""
    frames = [-1] * frame_count
    hits = 0
    faults = 0
    steps = []

    for idx, page in enumerate(pages):
        evicted = -1
        if page in frames:
            hits += 1
            is_hit = True
            f_idx = frames.index(page)
            action = f"Page {page} resident in Frame {f_idx + 1}"
        else:
            faults += 1
            is_hit = False
            if -1 in frames:
                empty_idx = frames.index(-1)
                frames[empty_idx] = page
                action = f"Loaded page {page} into Frame {empty_idx + 1}"
            else:
                # Clairvoyant lookahead: victim is page whose next use is farthest in future
                victim_idx = 0
                farthest = -1
                for f in range(frame_count):
                    resident = frames[f]
                    try:
                        next_use = pages.index(resident, idx + 1)
                    except ValueError:
                        next_use = -1  # Never used again
                    if next_use == -1:
                        victim_idx = f
                        break
                    if next_use > farthest:
                        farthest = next_use
                        victim_idx = f

                evicted = frames[victim_idx]
                frames[victim_idx] = page
                action = f"Replaced page {evicted} in Frame {victim_idx + 1} (MIN)"

        steps.append({
            "step": idx + 1,
            "page": page,
            "frames": list(frames),
            "is_hit": is_hit,
            "evicted": evicted,
            "action": action
        })

    total = len(pages)
    return {
        "algorithm": "Optimal",
        "total": total,
        "frames_count": frame_count,
        "hits": hits,
        "faults": faults,
        "hit_ratio": (hits / total) * 100.0 if total else 0.0,
        "fault_ratio": (faults / total) * 100.0 if total else 0.0,
        "steps": steps
    }


def print_trace(res: Dict[str, Any]) -> None:
    fc = res["frames_count"]
    print(f"\nSIMULATION TRACE: {res['algorithm']} ({fc} Frames, {res['total']} References)")
    frame_cols = "".join(f" Frame {f+1} |" for f in range(fc))
    print(f"| Step | Page  |{frame_cols} Status  | Action")
    print("|------|-------|" + "----------|" * fc + "---------|----------------------------------")
    for s in res["steps"]:
        row = f"| {s['step']:<4} | {s['page']:<5} |"
        for f in range(fc):
            v = s["frames"][f]
            row += f" {str(v) if v != -1 else '-':<8} |"
        st = "Hit" if s["is_hit"] else "Fault"
        row += f" {st:<7} | {s['action']}"
        print(row)


def print_summary(res: Dict[str, Any]) -> None:
    alg = res["algorithm"]
    print(f"\nPERFORMANCE SUMMARY ({alg})")
    print(f"| {'Metric':<32} | {'Output':<15} |")
    print(f"|{'-'*34}|{'-'*17}|")
    print(f"| {'Total page references':<32} | {res['total']:<15} |")
    print(f"| {f'Total page hits ({alg})':<32} | {res['hits']:<15} |")
    print(f"| {f'Total page faults ({alg})':<32} | {res['faults']:<15} |")
    print(f"| {f'Page hit ratio ({alg})':<32} | {res['hit_ratio']:6.2f}%         |")
    print(f"| {f'Page fault ratio ({alg})':<32} | {res['fault_ratio']:6.2f}%         |")


def print_comparison(results: List[Dict[str, Any]]) -> None:
    min_faults = min(r["faults"] for r in results)
    print("\n===============================================================================")
    print("  ALGORITHM COMPARATIVE EVALUATION SUMMARY")
    print("===============================================================================")
    print(f"| {'Algorithm':<12} | {'Page Hits':<10} | {'Page Faults':<12} | {'Hit Ratio':<12} | {'Fault Ratio':<12} |")
    print("|--------------|------------|--------------|--------------|--------------|")
    for r in results:
        best_tag = " <= BEST" if r["faults"] == min_faults else ""
        print(f"| {r['algorithm']:<12} | {r['hits']:<10} | {r['faults']:<12} | {r['hit_ratio']:9.2f}%   | {r['fault_ratio']:9.2f}%   |{best_tag}")
    print("===============================================================================\n")


def run_tests() -> bool:
    """Run verification tests against standard OS reference strings."""
    print("==> Running Page Replacement Verification Test Suite...")

    # Test 1: Silberschatz 10-reference sample
    ref1 = [7, 0, 1, 2, 0, 3, 0, 4, 2, 3]
    f_res = simulate_fifo(ref1, 3)
    l_res = simulate_lru(ref1, 3)
    o_res = simulate_optimal(ref1, 3)

    assert f_res["hits"] == 1 and f_res["faults"] == 9, f"FIFO failed: {f_res}"
    assert l_res["hits"] == 2 and l_res["faults"] == 8, f"LRU failed: {l_res}"
    assert o_res["hits"] == 4 and o_res["faults"] == 6, f"Optimal failed: {o_res}"
    print("[PASS] Test 1: Silberschatz reference string parity verified (FIFO:9, LRU:8, OPT:6 faults).")

    # Test 2: Belady's Anomaly verification
    ref2 = [1, 2, 3, 4, 1, 2, 5, 1, 2, 3, 4, 5]
    b3 = simulate_fifo(ref2, 3)
    b4 = simulate_fifo(ref2, 4)
    assert b3["faults"] == 9 and b4["faults"] == 10, f"Belady anomaly verification failed: {b3['faults']} vs {b4['faults']}"
    print(f"[PASS] Test 2: Belady's Anomaly confirmed on FIFO (3 frames = {b3['faults']} faults, 4 frames = {b4['faults']} faults).")

    # Test 3: Stack property check (LRU must never exhibit Belady's anomaly)
    l3 = simulate_lru(ref2, 3)
    l4 = simulate_lru(ref2, 4)
    assert l4["faults"] <= l3["faults"], f"LRU violated stack property: {l3['faults']} vs {l4['faults']}"
    print(f"[PASS] Test 3: LRU stack property confirmed (3 frames = {l3['faults']} faults, 4 frames = {l4['faults']} faults).")

    print("\nALL VERIFICATION TESTS PASSED SUCCESSFULLY (100% Correctness).")
    return True


def interactive_cli() -> None:
    print("===============================================================================")
    print("     VIRTUAL MEMORY SIMULATOR: PAGE REPLACEMENT ALGORITHMS (PBL TEAM 8)         ")
    print("===============================================================================\n")

    raw_ref = input("Enter the page-reference string:\n").strip()
    pages = parse_reference_string(raw_ref)

    fc_str = input("\nEnter the number of page frames:\n").strip()
    frame_count = int(fc_str)
    if frame_count < 1 or frame_count > 64:
        raise ValueError("Frames must be between 1 and 64.")

    print("\nSelect the algorithm:")
    print("1. FIFO")
    print("2. LRU")
    print("3. Optimal")
    print("4. Compare all algorithms\n")
    choice = input("Choice: ").strip()

    if choice == "1":
        r = simulate_fifo(pages, frame_count)
        print_trace(r)
        print_summary(r)
    elif choice == "2":
        r = simulate_lru(pages, frame_count)
        print_trace(r)
        print_summary(r)
    elif choice == "3":
        r = simulate_optimal(pages, frame_count)
        print_trace(r)
        print_summary(r)
    elif choice == "4":
        results = [
            simulate_fifo(pages, frame_count),
            simulate_lru(pages, frame_count),
            simulate_optimal(pages, frame_count)
        ]
        for r in results:
            print_trace(r)
            print_summary(r)
        print_comparison(results)
    else:
        print("Invalid choice.")


if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] == "--test":
        run_tests()
    elif len(sys.argv) > 1 and sys.argv[1] == "--demo":
        ref = [7, 0, 1, 2, 0, 3, 0, 4, 2, 3]
        results = [simulate_fifo(ref, 3), simulate_lru(ref, 3), simulate_optimal(ref, 3)]
        for r in results:
            print_trace(r)
            print_summary(r)
        print_comparison(results)
    elif len(sys.argv) > 1 and sys.argv[1] == "--belady":
        ref = [1, 2, 3, 4, 1, 2, 5, 1, 2, 3, 4, 5]
        b3 = simulate_fifo(ref, 3)
        b4 = simulate_fifo(ref, 4)
        print_trace(b3)
        print_summary(b3)
        print_trace(b4)
        print_summary(b4)
        print(f"\nBelady's Anomaly: 3 frames = {b3['faults']} faults, 4 frames = {b4['faults']} faults (increased!).")
    else:
        interactive_cli()
