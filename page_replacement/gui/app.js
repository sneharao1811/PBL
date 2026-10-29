(() => {
    "use strict";

    const form = document.getElementById("simulationForm");
    const referencesField = document.getElementById("referenceInput");
    const frameCountField = document.getElementById("frameCount");
    const algorithmField = document.getElementById("algorithmSelect");
    const errorField = document.getElementById("inputError");
    const resultsPanel = document.getElementById("resultsPanel");
    const summaryBody = document.getElementById("summaryBody");
    const comparisonNote = document.getElementById("comparisonNote");
    const faultChart = document.getElementById("faultChart");
    const traceAlgorithm = document.getElementById("traceAlgorithm");
    const traceHead = document.getElementById("traceHead");
    const traceBody = document.getElementById("traceBody");
    const workloadSummary = document.getElementById("workloadSummary");

    const ALGORITHM_ORDER = ["FIFO", "LRU", "Optimal"];
    const ALGORITHM_LABELS = {
        FIFO: "First In, First Out",
        LRU: "Least Recently Used",
        Optimal: "Optimal"
    };
    let latestResults = [];

    function parseReferences(rawValue) {
        const value = rawValue.trim();
        if (!value) throw new Error("Enter a page-reference string before running the simulation.");

        const tokens = value.split(/[\s,]+/);
        if (tokens.length > 500) throw new Error("Use no more than 500 page references per simulation.");

        const references = tokens.map((token) => {
            if (!/^\d+$/.test(token)) {
                throw new Error(`“${token}” is not a valid page number. Use non-negative whole numbers separated by spaces or commas.`);
            }
            const page = Number(token);
            if (!Number.isSafeInteger(page)) throw new Error("Page numbers must be safe whole-number values.");
            return page;
        });

        return references;
    }

    function parseFrameCount(rawValue) {
        if (rawValue.trim() === "") throw new Error("Enter the number of physical memory frames.");
        const frameCount = Number(rawValue);
        if (!Number.isInteger(frameCount) || frameCount < 1 || frameCount > 64) {
            throw new Error("Frame count must be a whole number from 1 to 64.");
        }
        return frameCount;
    }

    function simulate(references, frameCount, algorithm) {
        const frames = Array(frameCount).fill(null);
        const lastUsed = Array(frameCount).fill(-1);
        const steps = [];
        let fifoPointer = 0;
        let hits = 0;
        let faults = 0;

        references.forEach((page, index) => {
            const residentIndex = frames.indexOf(page);
            let status;
            let action;

            if (residentIndex !== -1) {
                hits += 1;
                status = "Hit";
                action = "Page already in memory";
                lastUsed[residentIndex] = index;
            } else {
                faults += 1;
                status = "Fault";
                let frameIndex = frames.indexOf(null);
                let evictedPage = null;

                if (frameIndex === -1) {
                    if (algorithm === "FIFO") {
                        frameIndex = fifoPointer;
                        fifoPointer = (fifoPointer + 1) % frameCount;
                    } else if (algorithm === "LRU") {
                        frameIndex = 0;
                        for (let candidate = 1; candidate < frameCount; candidate += 1) {
                            if (lastUsed[candidate] < lastUsed[frameIndex]) frameIndex = candidate;
                        }
                    } else {
                        frameIndex = chooseOptimalVictim(frames, references, index);
                    }
                    evictedPage = frames[frameIndex];
                }

                frames[frameIndex] = page;
                lastUsed[frameIndex] = index;
                action = evictedPage === null
                    ? `Loaded into Frame ${frameIndex + 1}`
                    : `Replaced page ${evictedPage} in Frame ${frameIndex + 1}`;
            }

            steps.push({ index: index + 1, page, frames: [...frames], status, action });
        });

        return {
            algorithm,
            hits,
            faults,
            total: references.length,
            hitRatio: (hits / references.length) * 100,
            faultRatio: (faults / references.length) * 100,
            steps
        };
    }

    function chooseOptimalVictim(frames, references, currentIndex) {
        let victimIndex = 0;
        let farthestNextUse = -1;

        for (let frameIndex = 0; frameIndex < frames.length; frameIndex += 1) {
            const residentPage = frames[frameIndex];
            const nextUse = references.indexOf(residentPage, currentIndex + 1);
            if (nextUse === -1) return frameIndex;
            if (nextUse > farthestNextUse) {
                victimIndex = frameIndex;
                farthestNextUse = nextUse;
            }
        }

        return victimIndex;
    }

    function formatRatio(value) {
        return `${value.toFixed(1)}%`;
    }

    function renderSummary(results, frameCount) {
        const fewestFaults = Math.min(...results.map((result) => result.faults));
        summaryBody.replaceChildren();

        results.forEach((result) => {
            const row = document.createElement("tr");
            if (result.faults === fewestFaults) row.className = "best-result";
            [result.algorithm, result.hits, result.faults, formatRatio(result.hitRatio), formatRatio(result.faultRatio)].forEach((value) => {
                const cell = document.createElement("td");
                cell.textContent = String(value);
                row.appendChild(cell);
            });
            summaryBody.appendChild(row);
        });

        const winners = results.filter((result) => result.faults === fewestFaults).map((result) => result.algorithm);
        const winnerText = results.length === 1
            ? `Showing ${results[0].algorithm} only. Select Compare all to compare page faults across algorithms.`
            : winners.length === results.length
                ? "All selected algorithms had the same number of faults."
                : `${winners.join(" and ")} had the fewest page faults for this reference string.`;
        comparisonNote.textContent = `${winnerText} Results depend on the supplied reference string and frame count.`;
        workloadSummary.textContent = `${results[0].total} references · ${frameCount} frames`;
    }

    function renderFaultChart(results) {
        const maxFaults = Math.max(1, ...results.map((result) => result.faults));
        const fewestFaults = Math.min(...results.map((result) => result.faults));
        faultChart.replaceChildren();
        faultChart.setAttribute("aria-label", results.map((result) => `${result.algorithm}: ${result.faults} page faults`).join("; "));

        results.forEach((result) => {
            const row = document.createElement("div");
            row.className = "chart-row";

            const label = document.createElement("span");
            label.className = "chart-label";
            label.textContent = result.algorithm;

            const track = document.createElement("span");
            track.className = "chart-track";
            const bar = document.createElement("span");
            bar.className = `chart-bar${result.faults === fewestFaults ? " is-best" : ""}`;
            bar.style.width = `${(result.faults / maxFaults) * 100}%`;
            track.appendChild(bar);

            const value = document.createElement("span");
            value.className = "chart-value";
            value.textContent = String(result.faults);

            row.append(label, track, value);
            faultChart.appendChild(row);
        });
    }

    function renderTraceSelector(results) {
        traceAlgorithm.replaceChildren();
        results.forEach((result) => {
            const option = document.createElement("option");
            option.value = result.algorithm;
            option.textContent = `${result.algorithm} · ${ALGORITHM_LABELS[result.algorithm]}`;
            traceAlgorithm.appendChild(option);
        });
        traceAlgorithm.value = results[0].algorithm;
    }

    function renderTrace(result) {
        const frameCount = result.steps[0].frames.length;
        const headRow = document.createElement("tr");
        ["Step", "Page", ...Array.from({ length: frameCount }, (_, index) => `Frame ${index + 1}`), "Result", "Action"].forEach((label) => {
            const cell = document.createElement("th");
            cell.scope = "col";
            cell.textContent = label;
            headRow.appendChild(cell);
        });
        traceHead.replaceChildren(headRow);
        traceBody.replaceChildren();

        result.steps.forEach((step) => {
            const row = document.createElement("tr");
            const stepCell = document.createElement("th");
            stepCell.scope = "row";
            stepCell.textContent = String(step.index);
            row.appendChild(stepCell);

            appendCell(row, step.page, "page-cell");
            step.frames.forEach((page) => appendCell(row, page === null ? "—" : page));

            const statusCell = document.createElement("td");
            statusCell.className = `status-cell ${step.status === "Hit" ? "status-hit" : "status-fault"}`;
            statusCell.textContent = step.status;
            row.appendChild(statusCell);
            appendCell(row, step.action, "action-cell");
            traceBody.appendChild(row);
        });
    }

    function appendCell(row, value, className = "") {
        const cell = document.createElement("td");
        if (className) cell.className = className;
        cell.textContent = String(value);
        row.appendChild(cell);
    }

    function runSimulation(event) {
        event.preventDefault();
        errorField.textContent = "";

        try {
            const references = parseReferences(referencesField.value);
            const frameCount = parseFrameCount(frameCountField.value);
            const selection = algorithmField.value;
            const algorithms = selection === "compare" ? ALGORITHM_ORDER : [selection];
            latestResults = algorithms.map((algorithm) => simulate(references, frameCount, algorithm));

            renderSummary(latestResults, frameCount);
            renderFaultChart(latestResults);
            renderTraceSelector(latestResults);
            renderTrace(latestResults[0]);
            resultsPanel.hidden = false;
        } catch (error) {
            errorField.textContent = error instanceof Error ? error.message : "Unable to run the simulation. Check the input values.";
            resultsPanel.hidden = true;
        }
    }

    form.addEventListener("submit", runSimulation);
    traceAlgorithm.addEventListener("change", () => {
        const result = latestResults.find((entry) => entry.algorithm === traceAlgorithm.value);
        if (result) renderTrace(result);
    });
    document.getElementById("loadSampleBtn").addEventListener("click", () => {
        referencesField.value = "7 0 1 2 0 3 0 4 2 3";
        frameCountField.value = "3";
        algorithmField.value = "compare";
        errorField.textContent = "";
        referencesField.focus();
    });
})();
