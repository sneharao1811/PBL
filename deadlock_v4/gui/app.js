/**
 * @file app.js
 * @brief Interactive Controller & Canvas RAG Engine for Deadlock Simulation System.
 * 
 * OS Project-Based Learning (PBL) - Team 8
 * Subsystem Lead: Sneha (UI & Frontend Lead)
 * Algorithmic Logic: Siddanth (Banker's Avoidance) & Rafath (Detection & Recovery)
 */

(function () {
    'use strict';

    // =========================================================================
    // 1. Data Model & System State
    // =========================================================================
    const state = {
        processes: [],
        resources: [],
        allocation: [],
        maximum: [],
        need: [],
        request: [],
        systemStatus: 'SAFE', // 'SAFE' | 'UNSAFE' | 'DEADLOCKED'
        safeSequence: [],
        deadlockedPids: [],
        stepper: {
            active: false,
            stepIndex: 0,
            work: [],
            finish: [],
            sequence: [],
            steps: [],
            timer: null
        }
    };

    // =========================================================================
    // 2. Presets Initializer
    // =========================================================================
    function loadTextbookSafePreset() {
        state.processes = [
            { id: 0, name: 'P0', active: true, priority: 1 },
            { id: 1, name: 'P1', active: true, priority: 2 },
            { id: 2, name: 'P2', active: true, priority: 3 },
            { id: 3, name: 'P3', active: true, priority: 4 },
            { id: 4, name: 'P4', active: true, priority: 5 }
        ];

        state.resources = [
            { id: 0, name: 'CPU (A)', total: 10, available: 3 },
            { id: 1, name: 'RAM (B)', total: 5, available: 3 },
            { id: 2, name: 'Disk (C)', total: 7, available: 2 }
        ];

        state.allocation = [
            [0, 1, 0],
            [2, 0, 0],
            [3, 0, 2],
            [2, 1, 1],
            [0, 0, 2]
        ];

        state.maximum = [
            [7, 5, 3],
            [3, 2, 2],
            [9, 0, 2],
            [2, 2, 2],
            [4, 3, 3]
        ];

        state.request = [
            [0, 0, 0],
            [0, 0, 0],
            [0, 0, 0],
            [0, 0, 0],
            [0, 0, 0]
        ];

        recalculateMatrices();
        addLogEntry('Loaded Textbook Safe Preset (Silberschatz OS: 5 processes, 3 resource types).', 'success');
    }

    function loadDeadlockPreset() {
        state.processes = [
            { id: 0, name: 'P0', active: true, priority: 1 },
            { id: 1, name: 'P1', active: true, priority: 2 },
            { id: 2, name: 'P2', active: true, priority: 3 }
        ];

        state.resources = [
            { id: 0, name: 'Mutex (R1)', total: 1, available: 0 },
            { id: 1, name: 'Tape (R2)', total: 1, available: 0 },
            { id: 2, name: 'Plotter (R3)', total: 1, available: 0 }
        ];

        // Circular wait: P0 holds R0 requests R1; P1 holds R1 requests R2; P2 holds R2 requests R0
        state.allocation = [
            [1, 0, 0],
            [0, 1, 0],
            [0, 0, 1]
        ];

        state.maximum = [
            [1, 1, 0],
            [0, 1, 1],
            [1, 0, 1]
        ];

        state.request = [
            [0, 1, 0],
            [0, 0, 1],
            [1, 0, 0]
        ];

        recalculateMatrices();
        addLogEntry('Loaded Circular Wait Deadlock Scenario. P0 -> R1 -> P1 -> R2 -> P2 -> R0 -> P0.', 'alert');
    }

    function loadUnsafePreset() {
        state.processes = [
            { id: 0, name: 'P0', active: true, priority: 1 },
            { id: 1, name: 'P1', active: true, priority: 2 },
            { id: 2, name: 'P2', active: true, priority: 3 },
            { id: 3, name: 'P3', active: true, priority: 4 }
        ];

        state.resources = [
            { id: 0, name: 'GPU (R1)', total: 6, available: 0 },
            { id: 1, name: 'Network (R2)', total: 6, available: 1 }
        ];

        state.allocation = [
            [2, 1],
            [1, 2],
            [2, 1],
            [1, 1]
        ];

        state.maximum = [
            [4, 3],
            [3, 4],
            [4, 3],
            [3, 3]
        ];

        state.request = [
            [0, 0],
            [0, 0],
            [0, 0],
            [0, 0]
        ];

        recalculateMatrices();
        addLogEntry('Loaded High Contention Scenario (Unsafe state threshold).', 'warning');
    }

    // =========================================================================
    // 3. Matrix Math & Recalculation
    // =========================================================================
    function recalculateMatrices() {
        const np = state.processes.length;
        const nr = state.resources.length;

        // Ensure rows & cols
        state.need = [];
        for (let i = 0; i < np; i++) {
            state.need[i] = [];
            for (let j = 0; j < nr; j++) {
                const alloc = state.allocation[i] && state.allocation[i][j] !== undefined ? state.allocation[i][j] : 0;
                const maxVal = state.maximum[i] && state.maximum[i][j] !== undefined ? state.maximum[i][j] : 0;
                state.need[i][j] = Math.max(0, maxVal - alloc);
            }
        }

        // Recalculate Available Vector
        for (let j = 0; j < nr; j++) {
            let allocatedSum = 0;
            for (let i = 0; i < np; i++) {
                if (state.processes[i].active && state.allocation[i]) {
                    allocatedSum += (state.allocation[i][j] || 0);
                }
            }
            state.resources[j].available = Math.max(0, state.resources[j].total - allocatedSum);
        }

        // Evaluate State
        evaluateSystemState();
        renderAllUI();
    }

    // =========================================================================
    // 4. Algorithmic Engines (Banker's Avoidance & Deadlock Detection)
    // =========================================================================
    function runBankersSafetyCheck(customState) {
        const s = customState || state;
        const np = s.processes.length;
        const nr = s.resources.length;

        const work = s.resources.map(r => r.available);
        const finish = s.processes.map(p => !p.active);
        const sequence = [];
        const steps = [];

        let activeCount = s.processes.filter(p => p.active).length;
        let progress = true;

        steps.push({
            type: 'init',
            work: [...work],
            finish: [...finish],
            message: `Initialized Work vector to Available [${work.join(', ')}].`
        });

        while (progress && sequence.length < activeCount) {
            progress = false;

            for (let i = 0; i < np; i++) {
                if (!finish[i] && s.processes[i].active) {
                    let canSatisfy = true;
                    for (let j = 0; j < nr; j++) {
                        if (s.need[i][j] > work[j]) {
                            canSatisfy = false;
                            break;
                        }
                    }

                    if (canSatisfy) {
                        for (let j = 0; j < nr; j++) {
                            work[j] += s.allocation[i][j];
                        }
                        finish[i] = true;
                        sequence.push(i);
                        progress = true;

                        steps.push({
                            type: 'process_done',
                            processId: i,
                            processName: s.processes[i].name,
                            work: [...work],
                            finish: [...finish],
                            sequence: [...sequence],
                            message: `Process ${s.processes[i].name} can finish! Reclaimed allocation. New Work = [${work.join(', ')}].`
                        });
                    }
                }
            }
        }

        const isSafe = sequence.length === activeCount;
        steps.push({
            type: isSafe ? 'safe_conclusion' : 'unsafe_conclusion',
            isSafe,
            sequence: [...sequence],
            message: isSafe
                ? `System is in a SAFE state. Safe sequence: < ${sequence.map(idx => s.processes[idx].name).join(' -> ')} >.`
                : `System is in an UNSAFE state. No process could complete with remaining resources.`
        });

        return { isSafe, sequence, steps };
    }

    function runDeadlockDetection(customState) {
        const s = customState || state;
        const np = s.processes.length;
        const nr = s.resources.length;

        const work = s.resources.map(r => r.available);
        const finish = [];

        for (let i = 0; i < np; i++) {
            if (!s.processes[i].active) {
                finish[i] = true;
                continue;
            }
            const holdsResources = s.allocation[i] && s.allocation[i].some(v => v > 0);
            finish[i] = !holdsResources;
        }

        let progress = true;
        while (progress) {
            progress = false;
            for (let i = 0; i < np; i++) {
                if (!finish[i] && s.processes[i].active) {
                    let canSatisfy = true;
                    for (let j = 0; j < nr; j++) {
                        const reqVal = (s.request[i] && s.request[i][j]) || 0;
                        if (reqVal > work[j]) {
                            canSatisfy = false;
                            break;
                        }
                    }

                    if (canSatisfy) {
                        for (let j = 0; j < nr; j++) {
                            work[j] += (s.allocation[i][j] || 0);
                        }
                        finish[i] = true;
                        progress = true;
                    }
                }
            }
        }

        const deadlockedPids = [];
        for (let i = 0; i < np; i++) {
            if (s.processes[i].active && !finish[i]) {
                deadlockedPids.push(i);
            }
        }

        const isDeadlocked = deadlockedPids.length > 0;
        return {
            isDeadlocked,
            deadlockedPids,
            explanation: isDeadlocked
                ? `Deadlock detected! Processes [${deadlockedPids.map(p => s.processes[p].name).join(', ')}] are in permanent circular wait.`
                : `No deadlock. All processes can acquire current requests.`
        };
    }

    function evaluateSystemState() {
        const det = runDeadlockDetection();
        if (det.isDeadlocked) {
            state.systemStatus = 'DEADLOCKED';
            state.deadlockedPids = det.deadlockedPids;
            state.safeSequence = [];
            return;
        }

        state.deadlockedPids = [];
        const safety = runBankersSafetyCheck();
        if (safety.isSafe) {
            state.systemStatus = 'SAFE';
            state.safeSequence = safety.sequence;
        } else {
            state.systemStatus = 'UNSAFE';
            state.safeSequence = [];
        }
    }

    // =========================================================================
    // 5. Deadlock Recovery Engine
    // =========================================================================
    function executeRecoveryIntervention(policy) {
        const det = runDeadlockDetection();
        if (!det.isDeadlocked) {
            addLogEntry('Recovery skipped: No deadlock currently present.', 'info');
            return;
        }

        if (policy === 'terminate-all') {
            det.deadlockedPids.forEach(pid => {
                for (let j = 0; j < state.resources.length; j++) {
                    state.allocation[pid][j] = 0;
                    state.maximum[pid][j] = 0;
                    state.request[pid][j] = 0;
                }
                state.processes[pid].active = false;
            });
            addLogEntry(`RECOVERY: Aborted all ${det.deadlockedPids.length} deadlocked processes.`, 'success');
        } else if (policy === 'one-by-one') {
            // Select victim holding the most resources
            let bestVictim = -1;
            let maxHeld = -1;
            det.deadlockedPids.forEach(pid => {
                const totalHeld = state.allocation[pid].reduce((a, b) => a + b, 0);
                if (totalHeld > maxHeld) {
                    maxHeld = totalHeld;
                    bestVictim = pid;
                }
            });

            if (bestVictim >= 0) {
                const victimName = state.processes[bestVictim].name;
                for (let j = 0; j < state.resources.length; j++) {
                    state.allocation[bestVictim][j] = 0;
                    state.maximum[bestVictim][j] = 0;
                    state.request[bestVictim][j] = 0;
                }
                state.processes[bestVictim].active = false;
                addLogEntry(`RECOVERY: Selectively aborted victim ${victimName} (reclaimed ${maxHeld} units).`, 'success');
            }
        } else if (policy === 'preempt') {
            let victim = det.deadlockedPids[0];
            const victimName = state.processes[victim].name;
            for (let j = 0; j < state.resources.length; j++) {
                state.allocation[victim][j] = 0;
                state.request[victim][j] = 0;
            }
            addLogEntry(`RECOVERY (PREEMPTION): Preempted resources from ${victimName} and rolled back transactions.`, 'warning');
        }

        recalculateMatrices();
    }

    // =========================================================================
    // 6. Canvas RAG (Resource Allocation Graph) Renderer
    // =========================================================================
    const canvas = document.getElementById('ragCanvas');
    const ctx = canvas.getContext('2d');
    let animationFrameId = null;
    let pulseTime = 0;

    function resizeCanvas() {
        const container = document.getElementById('canvasContainer');
        const rect = container.getBoundingClientRect();
        const dpr = window.devicePixelRatio || 1;
        canvas.width = rect.width * dpr;
        canvas.height = rect.height * dpr;
        ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
    }

    function renderRAG() {
        const container = document.getElementById('canvasContainer');
        const width = container.clientWidth;
        const height = container.clientHeight;

        ctx.clearRect(0, 0, width, height);

        // Grid background effect
        ctx.strokeStyle = 'rgba(38, 58, 76, 0.055)';
        ctx.lineWidth = 1;
        const gridSize = 28;
        for (let x = 0; x < width; x += gridSize) {
            ctx.beginPath();
            ctx.moveTo(x, 0);
            ctx.lineTo(x, height);
            ctx.stroke();
        }
        for (let y = 0; y < height; y += gridSize) {
            ctx.beginPath();
            ctx.moveTo(0, y);
            ctx.lineTo(width, y);
            ctx.stroke();
        }

        const np = state.processes.length;
        const nr = state.resources.length;
        if (np === 0 || nr === 0) return;

        // Position Process Nodes on the Left/Top and Resource Nodes on Right/Bottom
        const processNodes = [];
        const resourceNodes = [];

        const leftX = width * 0.25;
        const rightX = width * 0.75;
        const pStepY = height / (np + 1);
        const rStepY = height / (nr + 1);

        for (let i = 0; i < np; i++) {
            processNodes.push({
                id: i,
                name: state.processes[i].name,
                x: leftX,
                y: pStepY * (i + 1),
                radius: 22,
                active: state.processes[i].active,
                isDeadlocked: state.deadlockedPids.includes(i)
            });
        }

        for (let j = 0; j < nr; j++) {
            resourceNodes.push({
                id: j,
                name: state.resources[j].name,
                x: rightX,
                y: rStepY * (j + 1),
                w: 78,
                h: 46,
                total: state.resources[j].total,
                available: state.resources[j].available
            });
        }

        // Draw Edges First (Behind nodes)
        // 1. Assignment Edges: Resource -> Process (Solid Green/Cyan Arrow)
        for (let i = 0; i < np; i++) {
            if (!state.processes[i].active) continue;
            for (let j = 0; j < nr; j++) {
                const allocUnits = state.allocation[i] ? state.allocation[i][j] : 0;
                if (allocUnits > 0) {
                    const rNode = resourceNodes[j];
                    const pNode = processNodes[i];
                    drawDirectedEdge(rNode.x - rNode.w / 2, rNode.y, pNode.x + pNode.radius, pNode.y, {
                        color: pNode.isDeadlocked ? '#ef4444' : '#10b981',
                        label: `${allocUnits} unit${allocUnits > 1 ? 's' : ''}`,
                        dashed: false,
                        curveOffset: (j - i) * 6
                    });
                }
            }
        }

        // 2. Request Edges: Process -> Resource (Dashed Amber/Red Arrow)
        for (let i = 0; i < np; i++) {
            if (!state.processes[i].active) continue;
            for (let j = 0; j < nr; j++) {
                const reqUnits = state.request[i] ? state.request[i][j] : 0;
                if (reqUnits > 0) {
                    const pNode = processNodes[i];
                    const rNode = resourceNodes[j];
                    drawDirectedEdge(pNode.x + pNode.radius, pNode.y, rNode.x - rNode.w / 2, rNode.y, {
                        color: pNode.isDeadlocked ? '#ef4444' : '#f59e0b',
                        label: `Req ${reqUnits}`,
                        dashed: true,
                        curveOffset: (i - j) * 6
                    });
                }
            }
        }

        // Draw Process Nodes (Circles)
        processNodes.forEach(p => {
            ctx.save();
            ctx.beginPath();
            ctx.arc(p.x, p.y, p.radius, 0, Math.PI * 2);

            if (!p.active) {
                ctx.fillStyle = '#eef1f4';
                ctx.strokeStyle = '#b9c3cc';
            } else if (p.isDeadlocked) {
                const glow = Math.sin(pulseTime * 4) * 8 + 14;
                ctx.shadowColor = '#ef4444';
                ctx.shadowBlur = Math.min(glow / 3, 6);
                ctx.fillStyle = 'rgba(165, 59, 56, 0.12)';
                ctx.strokeStyle = '#ef4444';
            } else {
                ctx.shadowColor = '#35749e';
                ctx.shadowBlur = 3;
                ctx.fillStyle = 'rgba(53, 116, 158, 0.10)';
                ctx.strokeStyle = '#35749e';
            }
            ctx.lineWidth = 2.5;
            ctx.fill();
            ctx.stroke();
            ctx.restore();

            // Label
            ctx.fillStyle = p.active ? '#17212b' : '#6c7a87';
            ctx.font = '600 12px "JetBrains Mono", monospace';
            ctx.textAlign = 'center';
            ctx.textBaseline = 'middle';
            ctx.fillText(p.name, p.x, p.y - (p.active ? 0 : -1));

            if (!p.active) {
                ctx.font = '500 9px sans-serif';
                ctx.fillStyle = '#ef4444';
                ctx.fillText('TERMINATED', p.x, p.y + p.radius + 12);
            }
        });

        // Draw Resource Nodes (Rectangles with instance dots)
        resourceNodes.forEach(r => {
            const rx = r.x - r.w / 2;
            const ry = r.y - r.h / 2;

            ctx.save();
            ctx.shadowColor = '#a855f7';
            ctx.shadowBlur = 2;
            ctx.fillStyle = 'rgba(128, 98, 154, 0.08)';
            ctx.strokeStyle = '#80629a';
            ctx.lineWidth = 2;

            roundRect(ctx, rx, ry, r.w, r.h, 8);
            ctx.fill();
            ctx.stroke();
            ctx.restore();

            // Header Name
            ctx.fillStyle = '#17212b';
            ctx.font = '600 11px "Outfit", sans-serif';
            ctx.textAlign = 'center';
            ctx.textBaseline = 'top';
            ctx.fillText(r.name, r.x, ry + 6);

            // Resource Instances (Dots)
            const dotCount = Math.min(r.total, 8); // Render up to 8 visual dots
            const dotSpacing = r.w / (dotCount + 1);
            const inUse = r.total - r.available;

            for (let d = 0; d < dotCount; d++) {
                const dotX = rx + (d + 1) * dotSpacing;
                const dotY = ry + r.h - 14;
                ctx.beginPath();
                ctx.arc(dotX, dotY, 3.5, 0, Math.PI * 2);

                if (d < inUse) {
                    ctx.fillStyle = '#10b981'; // In use (allocated)
                } else {
                    ctx.fillStyle = '#aab5bf'; // Free instance
                }
                ctx.fill();
            }

            // Available count badge
            ctx.fillStyle = '#52616e';
            ctx.font = '500 9px "JetBrains Mono", monospace';
            ctx.fillText(`Avail: ${r.available}/${r.total}`, r.x, ry + r.h + 8);
        });
    }

    function drawDirectedEdge(x1, y1, x2, y2, opts) {
        ctx.save();
        ctx.beginPath();
        if (opts.dashed) {
            ctx.setLineDash([4, 4]);
        }
        ctx.strokeStyle = opts.color;
        ctx.lineWidth = 1.8;

        const midX = (x1 + x2) / 2;
        const midY = (y1 + y2) / 2 + (opts.curveOffset || 0);

        ctx.moveTo(x1, y1);
        ctx.quadraticCurveTo(midX, midY, x2, y2);
        ctx.stroke();

        // Arrow head calculation
        const t = 0.95;
        const arrowX = (1 - t) * (1 - t) * x1 + 2 * (1 - t) * t * midX + t * t * x2;
        const arrowY = (1 - t) * (1 - t) * y1 + 2 * (1 - t) * t * midY + t * t * y2;
        const angle = Math.atan2(y2 - midY, x2 - midX);

        ctx.setLineDash([]);
        ctx.fillStyle = opts.color;
        ctx.beginPath();
        ctx.moveTo(x2, y2);
        ctx.lineTo(x2 - 10 * Math.cos(angle - Math.PI / 6), y2 - 10 * Math.sin(angle - Math.PI / 6));
        ctx.lineTo(x2 - 10 * Math.cos(angle + Math.PI / 6), y2 - 10 * Math.sin(angle + Math.PI / 6));
        ctx.closePath();
        ctx.fill();

        // Label
        if (opts.label) {
            ctx.fillStyle = opts.color;
            ctx.font = '600 9px "JetBrains Mono", monospace';
            ctx.textAlign = 'center';
            ctx.fillText(opts.label, midX, midY - 6);
        }

        ctx.restore();
    }

    function roundRect(ctx, x, y, width, height, radius) {
        ctx.beginPath();
        ctx.moveTo(x + radius, y);
        ctx.lineTo(x + width - radius, y);
        ctx.quadraticCurveTo(x + width, y, x + width, y + radius);
        ctx.lineTo(x + width, y + height - radius);
        ctx.quadraticCurveTo(x + width, y + height, x + width - radius, y + height);
        ctx.lineTo(x + radius, y + height);
        ctx.quadraticCurveTo(x, y + height, x, y + height - radius);
        ctx.lineTo(x, y + radius);
        ctx.quadraticCurveTo(x, y, x + radius, y);
        ctx.closePath();
    }

    function startAnimationLoop() {
        function loop() {
            pulseTime += 0.03;
            renderRAG();
            animationFrameId = requestAnimationFrame(loop);
        }
        loop();
    }

    // =========================================================================
    // 7. Dynamic Table & UI View Builders
    // =========================================================================
    function renderMatrixTables() {
        const np = state.processes.length;
        const nr = state.resources.length;

        // 1. Available & Total Vector
        let vecHtml = `<table class="matrix-table"><thead><tr><th>Vector</th>`;
        state.resources.forEach(r => vecHtml += `<th>${r.name}</th>`);
        vecHtml += `</tr></thead><tbody>`;

        // Total
        vecHtml += `<tr><td><strong>Total (E)</strong></td>`;
        state.resources.forEach((r, j) => {
            vecHtml += `<td><input type="number" min="0" value="${r.total}" data-type="total" data-res="${j}"></td>`;
        });
        vecHtml += `</tr>`;

        // Available (computed)
        vecHtml += `<tr><td><strong>Available (A)</strong></td>`;
        state.resources.forEach(r => {
            vecHtml += `<td><strong style="color:var(--accent-cyan);">${r.available}</strong></td>`;
        });
        vecHtml += `</tr></tbody></table>`;
        document.getElementById('vectorTableWrap').innerHTML = vecHtml;

        // 2. Allocation Matrix
        let allocHtml = `<table class="matrix-table"><thead><tr><th>PID</th>`;
        state.resources.forEach(r => allocHtml += `<th>${r.name}</th>`);
        allocHtml += `</tr></thead><tbody>`;
        state.processes.forEach((p, i) => {
            allocHtml += `<tr style="${p.active ? '' : 'opacity:0.4;'}"><td><strong>${p.name}</strong></td>`;
            state.resources.forEach((r, j) => {
                const val = (state.allocation[i] && state.allocation[i][j]) || 0;
                allocHtml += `<td><input type="number" min="0" value="${val}" data-type="alloc" data-row="${i}" data-col="${j}" ${p.active ? '' : 'disabled'}></td>`;
            });
            allocHtml += `</tr>`;
        });
        allocHtml += `</tbody></table>`;
        document.getElementById('allocTableWrap').innerHTML = allocHtml;

        // 3. Maximum Claim Matrix
        let maxHtml = `<table class="matrix-table"><thead><tr><th>PID</th>`;
        state.resources.forEach(r => maxHtml += `<th>${r.name}</th>`);
        maxHtml += `</tr></thead><tbody>`;
        state.processes.forEach((p, i) => {
            maxHtml += `<tr style="${p.active ? '' : 'opacity:0.4;'}"><td><strong>${p.name}</strong></td>`;
            state.resources.forEach((r, j) => {
                const val = (state.maximum[i] && state.maximum[i][j]) || 0;
                maxHtml += `<td><input type="number" min="0" value="${val}" data-type="max" data-row="${i}" data-col="${j}" ${p.active ? '' : 'disabled'}></td>`;
            });
            maxHtml += `</tr>`;
        });
        maxHtml += `</tbody></table>`;
        document.getElementById('maxTableWrap').innerHTML = maxHtml;

        // 4. Need Matrix (Readonly auto-computed)
        let needHtml = `<table class="matrix-table"><thead><tr><th>PID</th>`;
        state.resources.forEach(r => needHtml += `<th>${r.name}</th>`);
        needHtml += `</tr></thead><tbody>`;
        state.processes.forEach((p, i) => {
            needHtml += `<tr style="${p.active ? '' : 'opacity:0.4;'}"><td><strong>${p.name}</strong></td>`;
            state.resources.forEach((r, j) => {
                const val = (state.need[i] && state.need[i][j]) || 0;
                needHtml += `<td><strong style="color:var(--text-secondary);">${val}</strong></td>`;
            });
            needHtml += `</tr>`;
        });
        needHtml += `</tbody></table>`;
        document.getElementById('needTableWrap').innerHTML = needHtml;

        // 5. Outstanding Requests Matrix
        let reqHtml = `<table class="matrix-table"><thead><tr><th>PID</th>`;
        state.resources.forEach(r => reqHtml += `<th>${r.name}</th>`);
        reqHtml += `</tr></thead><tbody>`;
        state.processes.forEach((p, i) => {
            reqHtml += `<tr style="${p.active ? '' : 'opacity:0.4;'}"><td><strong>${p.name}</strong></td>`;
            state.resources.forEach((r, j) => {
                const val = (state.request[i] && state.request[i][j]) || 0;
                reqHtml += `<td><input type="number" min="0" value="${val}" data-type="req" data-row="${i}" data-col="${j}" ${p.active ? '' : 'disabled'}></td>`;
            });
            reqHtml += `</tr>`;
        });
        reqHtml += `</tbody></table>`;
        document.getElementById('reqTableWrap').innerHTML = reqHtml;

        bindMatrixInputListeners();
    }

    function bindMatrixInputListeners() {
        document.querySelectorAll('table.matrix-table input').forEach(input => {
            input.addEventListener('change', e => {
                const target = e.target;
                const type = target.dataset.type;
                const val = parseInt(target.value, 10) || 0;

                if (type === 'total') {
                    const resIdx = parseInt(target.dataset.res, 10);
                    state.resources[resIdx].total = val;
                } else if (type === 'alloc') {
                    const r = parseInt(target.dataset.row, 10);
                    const c = parseInt(target.dataset.col, 10);
                    state.allocation[r][c] = val;
                } else if (type === 'max') {
                    const r = parseInt(target.dataset.row, 10);
                    const c = parseInt(target.dataset.col, 10);
                    state.maximum[r][c] = val;
                } else if (type === 'req') {
                    const r = parseInt(target.dataset.row, 10);
                    const c = parseInt(target.dataset.col, 10);
                    state.request[r][c] = val;
                }

                recalculateMatrices();
            });
        });
    }

    function renderStatusBadges() {
        const badge = document.getElementById('systemStateBadge');
        const text = document.getElementById('systemStateText');

        badge.className = 'state-badge';

        if (state.systemStatus === 'SAFE') {
            badge.classList.add('state-safe');
            text.textContent = 'SYSTEM SAFE';
        } else if (state.systemStatus === 'UNSAFE') {
            badge.classList.add('state-unsafe');
            text.textContent = 'SYSTEM UNSAFE';
        } else {
            badge.classList.add('state-deadlock');
            text.textContent = 'DEADLOCK DETECTED';
        }

        // Safe Sequence Strip
        const pillsWrap = document.getElementById('safeSeqPills');
        if (state.safeSequence.length > 0) {
            let pillsHtml = '';
            state.safeSequence.forEach((idx, k) => {
                pillsHtml += `<span class="seq-pill">${state.processes[idx].name}</span>`;
                if (k < state.safeSequence.length - 1) {
                    pillsHtml += `<span class="seq-arrow">&rarr;</span>`;
                }
            });
            pillsWrap.innerHTML = pillsHtml;
        } else {
            pillsWrap.innerHTML = `<span style="font-size:0.75rem; color:var(--text-muted);">None (System is in ${state.systemStatus} state)</span>`;
        }

        // Utilization bars
        renderUtilizationBars();

        // Request Process Select
        const sel = document.getElementById('reqProcessSelect');
        sel.innerHTML = '';
        state.processes.filter(p => p.active).forEach(p => {
            const opt = document.createElement('option');
            opt.value = p.id;
            opt.textContent = `${p.name} (Active)`;
            sel.appendChild(opt);
        });

        // Request inputs grid
        const reqGrid = document.getElementById('requestInputsGrid');
        let gridHtml = '';
        state.resources.forEach((r, j) => {
            gridHtml += `<div class="req-input-col">
                <span>${r.name}</span>
                <input type="number" min="0" value="0" id="reqInputRes_${j}">
            </div>`;
        });
        reqGrid.innerHTML = gridHtml;
    }

    function renderUtilizationBars() {
        const container = document.getElementById('utilizationBarsContainer');
        let html = '';
        let hasBottleneck = false;

        state.resources.forEach(r => {
            const allocated = r.total - r.available;
            const pct = r.total > 0 ? ((allocated / r.total) * 100).toFixed(1) : 0;
            const isHigh = pct >= 80;
            if (isHigh) hasBottleneck = true;

            html += `<div class="util-item">
                <div class="util-meta">
                    <span class="util-name">${r.name}</span>
                    <span class="util-pct">${allocated}/${r.total} (${pct}%)</span>
                </div>
                <div class="util-track">
                    <div class="util-fill ${isHigh ? 'fill-high' : ''}" style="width: ${pct}%;"></div>
                </div>
            </div>`;
        });
        container.innerHTML = html;

        const notice = document.getElementById('bottleneckNotice');
        if (hasBottleneck) {
            notice.className = 'bottleneck-tag bottleneck-alert';
            notice.textContent = 'High Contention / Bottleneck';
        } else {
            notice.className = 'bottleneck-tag';
            notice.textContent = 'Normal Headroom';
        }
    }

    function renderAllUI() {
        renderMatrixTables();
        renderStatusBadges();
        updateDetectionUI();
    }

    function updateDetectionUI() {
        const box = document.getElementById('detectionVerdictBox');
        const icon = document.getElementById('detectionIcon');
        const heading = document.getElementById('detectionHeading');
        const detail = document.getElementById('detectionDetail');

        if (state.systemStatus === 'DEADLOCKED') {
            box.className = 'detection-verdict-box alert-active';
            icon.textContent = '!';
            heading.textContent = `Deadlock Detected (${state.deadlockedPids.length} processes)`;
            detail.textContent = `Processes [${state.deadlockedPids.map(p => state.processes[p].name).join(', ')}] are in a mutual circular hold-and-wait state.`;
        } else {
            box.className = 'detection-verdict-box';
            icon.textContent = 'OK';
            heading.textContent = 'No Deadlock Detected';
            detail.textContent = 'All currently requested and active resources can execute safely.';
        }
    }

    function addLogEntry(msg, type = 'info') {
        const stream = document.getElementById('logStream');
        const entry = document.createElement('div');
        const timeStr = new Date().toLocaleTimeString();

        entry.className = `log-entry log-${type}`;
        entry.innerHTML = `<span class="log-time">[${timeStr}]</span> ${msg}`;
        stream.appendChild(entry);
        stream.scrollTop = stream.scrollHeight;
    }

    // =========================================================================
    // 8. Event Handlers & Stepper Controller
    // =========================================================================
    function initEventHandlers() {
        // Tab switching
        document.querySelectorAll('.tab-btn').forEach(btn => {
            btn.addEventListener('click', () => {
                document.querySelectorAll('.tab-btn').forEach(b => b.classList.remove('active'));
                document.querySelectorAll('.tab-content').forEach(c => c.classList.remove('active'));
                btn.classList.add('active');
                const tabId = `tab-${btn.dataset.tab}`;
                document.getElementById(tabId).classList.add('active');
            });
        });

        // Presets
        document.getElementById('presetSafeBtn').addEventListener('click', loadTextbookSafePreset);
        document.getElementById('presetDeadlockBtn').addEventListener('click', loadDeadlockPreset);
        document.getElementById('presetUnsafeBtn').addEventListener('click', loadUnsafePreset);
        document.getElementById('resetSysBtn').addEventListener('click', () => {
            state.processes.forEach(p => p.active = true);
            state.allocation.forEach(row => row.fill(0));
            state.maximum.forEach(row => row.fill(0));
            state.request.forEach(row => row.fill(0));
            recalculateMatrices();
            addLogEntry('System reset to blank state.', 'warning');
        });

        // Add / Remove Process
        document.getElementById('addProcessBtn').addEventListener('click', () => {
            const newId = state.processes.length;
            state.processes.push({ id: newId, name: `P${newId}`, active: true, priority: newId + 1 });
            state.allocation.push(new Array(state.resources.length).fill(0));
            state.maximum.push(new Array(state.resources.length).fill(0));
            state.request.push(new Array(state.resources.length).fill(0));
            recalculateMatrices();
            addLogEntry(`Added process P${newId}.`, 'info');
        });

        document.getElementById('removeProcessBtn').addEventListener('click', () => {
            if (state.processes.length <= 1) return;
            const removed = state.processes.pop();
            state.allocation.pop();
            state.maximum.pop();
            state.request.pop();
            recalculateMatrices();
            addLogEntry(`Removed process ${removed.name}.`, 'info');
        });

        // Add / Remove Resource
        document.getElementById('addResourceBtn').addEventListener('click', () => {
            const newId = state.resources.length;
            const resName = `R${newId}`;
            state.resources.push({ id: newId, name: resName, total: 4, available: 4 });
            state.allocation.forEach(row => row.push(0));
            state.maximum.forEach(row => row.push(0));
            state.request.forEach(row => row.push(0));
            recalculateMatrices();
            addLogEntry(`Added resource type ${resName} with 4 capacity.`, 'info');
        });

        document.getElementById('removeResourceBtn').addEventListener('click', () => {
            if (state.resources.length <= 1) return;
            const removed = state.resources.pop();
            state.allocation.forEach(row => row.pop());
            state.maximum.forEach(row => row.pop());
            state.request.forEach(row => row.pop());
            recalculateMatrices();
            addLogEntry(`Removed resource type ${removed.name}.`, 'info');
        });

        document.getElementById('recalculateBtn').addEventListener('click', () => {
            recalculateMatrices();
            addLogEntry('Recalculated Need matrix and Available vectors.', 'success');
        });

        // Banker's Algorithm Tab
        document.getElementById('runBankersBtn').addEventListener('click', () => {
            const res = runBankersSafetyCheck();
            const msgBox = document.getElementById('bankersVerdictMsg');
            if (res.isSafe) {
                msgBox.innerHTML = `<strong style="color:var(--state-safe);">SYSTEM IS SAFE</strong><br>` +
                    `Valid Safe Execution Sequence: &lt; ${res.sequence.map(i => state.processes[i].name).join(' &rarr; ')} &gt;.<br>` +
                    `Every active process can be guaranteed completion without entering deadlock.`;
                addLogEntry(`Banker's Algorithm: Safe Sequence found < ${res.sequence.map(i => state.processes[i].name).join(' -> ')} >.`, 'success');
            } else {
                msgBox.innerHTML = `<strong style="color:var(--state-unsafe);">SYSTEM IS IN AN UNSAFE STATE</strong><br>` +
                    `No process can be satisfied with the remaining Available resources. An imminent deadlock may occur under maximum demand.`;
                addLogEntry(`Banker's Algorithm: System is in an UNSAFE state.`, 'warning');
            }
        });

        // Stepper
        const stepDisplay = document.getElementById('stepperDisplay');
        const stepWorkVal = document.getElementById('stepWorkVal');
        const stepExplanation = document.getElementById('stepExplanation');
        const stepChips = document.getElementById('stepProcessChips');

        function initStepper() {
            const check = runBankersSafetyCheck();
            state.stepper.active = true;
            state.stepper.stepIndex = 0;
            state.stepper.steps = check.steps;

            stepWorkVal.textContent = `[ ${state.resources.map(r => r.available).join(', ')} ]`;
            stepExplanation.textContent = 'Stepper initialized. Click "Next Step" to start evaluation.';

            let chipsHtml = '';
            state.processes.filter(p => p.active).forEach(p => {
                chipsHtml += `<span class="chip" id="chip_p_${p.id}">${p.name}</span>`;
            });
            stepChips.innerHTML = chipsHtml;
        }

        document.getElementById('stepResetBtn').addEventListener('click', () => {
            if (state.stepper.timer) clearInterval(state.stepper.timer);
            initStepper();
        });

        document.getElementById('stepNextBtn').addEventListener('click', () => {
            if (!state.stepper.steps || state.stepper.steps.length === 0) {
                initStepper();
            }

            state.stepper.stepIndex++;
            if (state.stepper.stepIndex >= state.stepper.steps.length) {
                stepExplanation.textContent = 'Stepper reached conclusion of safety sequence evaluation.';
                if (state.stepper.timer) clearInterval(state.stepper.timer);
                return;
            }

            const step = state.stepper.steps[state.stepper.stepIndex];
            stepWorkVal.textContent = `[ ${step.work.join(', ')} ]`;
            stepExplanation.textContent = step.message;

            if (step.processId !== undefined) {
                const chip = document.getElementById(`chip_p_${step.processId}`);
                if (chip) chip.className = 'chip chip-done';
            }
        });

        document.getElementById('stepAutoBtn').addEventListener('click', () => {
            initStepper();
            if (state.stepper.timer) clearInterval(state.stepper.timer);
            state.stepper.timer = setInterval(() => {
                document.getElementById('stepNextBtn').click();
                if (state.stepper.stepIndex >= state.stepper.steps.length - 1) {
                    clearInterval(state.stepper.timer);
                }
            }, 1200);
        });

        // Detection & Recovery Tab
        document.getElementById('runDetectionBtn').addEventListener('click', () => {
            evaluateSystemState();
            updateDetectionUI();
            if (state.systemStatus === 'DEADLOCKED') {
                addLogEntry(`Deadlock Check: DEADLOCK DETECTED on PIDs [${state.deadlockedPids.map(p => state.processes[p].name).join(', ')}].`, 'alert');
            } else {
                addLogEntry('Deadlock Check: NO deadlock detected.', 'success');
            }
        });

        document.getElementById('executeRecoveryBtn').addEventListener('click', () => {
            const selectedPolicy = document.querySelector('input[name="recoveryPolicy"]:checked').value;
            executeRecoveryIntervention(selectedPolicy);
        });

        // Dynamic Request Submission
        document.getElementById('submitRequestBtn').addEventListener('click', () => {
            const pid = parseInt(document.getElementById('reqProcessSelect').value, 10);
            const reqVec = [];
            for (let j = 0; j < state.resources.length; j++) {
                const inp = document.getElementById(`reqInputRes_${j}`);
                reqVec.push(parseInt(inp.value, 10) || 0);
            }

            const procName = state.processes[pid].name;
            const feedback = document.getElementById('dispatchFeedbackMsg');

            // 1. Check Request <= Need
            for (let j = 0; j < state.resources.length; j++) {
                if (reqVec[j] > state.need[pid][j]) {
                    feedback.innerHTML = `<span style="color:var(--state-deadlock);">DENIED:</span> ${procName} requested ${reqVec[j]} of ${state.resources[j].name}, which exceeds its declared maximum need (${state.need[pid][j]}).`;
                    addLogEntry(`Request by ${procName} rejected: Exceeded declared need.`, 'alert');
                    return;
                }
            }

            // 2. Check Request <= Available
            for (let j = 0; j < state.resources.length; j++) {
                if (reqVec[j] > state.resources[j].available) {
                    // Record pending request
                    state.request[pid][j] = reqVec[j];
                    recalculateMatrices();
                    feedback.innerHTML = `<span style="color:var(--state-unsafe);">BLOCKED:</span> Not enough ${state.resources[j].name} available (requested ${reqVec[j]}, free: ${state.resources[j].available}). Request queued.`;
                    addLogEntry(`Request by ${procName} blocked: Process waiting on resource headroom.`, 'warning');
                    return;
                }
            }

            // 3. Hypothetical Banker's check
            const clone = JSON.parse(JSON.stringify(state));
            for (let j = 0; j < state.resources.length; j++) {
                clone.resources[j].available -= reqVec[j];
                clone.allocation[pid][j] += reqVec[j];
                clone.need[pid][j] -= reqVec[j];
                clone.request[pid][j] = 0;
            }

            const safety = runBankersSafetyCheck(clone);
            if (safety.isSafe) {
                // Grant!
                for (let j = 0; j < state.resources.length; j++) {
                    state.allocation[pid][j] += reqVec[j];
                    state.request[pid][j] = 0;
                }
                recalculateMatrices();
                feedback.innerHTML = `<span style="color:var(--state-safe);">GRANTED:</span> Banker's Avoidance approved request. Safe sequence preserved: &lt; ${safety.sequence.map(i => state.processes[i].name).join(' &rarr; ')} &gt;.`;
                addLogEntry(`Request by ${procName} GRANTED [${reqVec.join(', ')}]. Safe sequence verified.`, 'success');
            } else {
                state.request[pid] = [...reqVec];
                recalculateMatrices();
                feedback.innerHTML = `<span style="color:var(--state-unsafe);">AVOIDANCE REJECTION:</span> Granting request would transition system into an UNSAFE state (potential deadlock). Request deferred.`;
                addLogEntry(`Request by ${procName} DENIED by Banker's Algorithm (Avoidance rule triggered).`, 'warning');
            }
        });

        // Release Resources
        document.getElementById('releaseResourceBtn').addEventListener('click', () => {
            const pid = parseInt(document.getElementById('reqProcessSelect').value, 10);
            const procName = state.processes[pid].name;
            for (let j = 0; j < state.resources.length; j++) {
                const inp = document.getElementById(`reqInputRes_${j}`);
                const relVal = parseInt(inp.value, 10) || 0;
                if (relVal <= state.allocation[pid][j]) {
                    state.allocation[pid][j] -= relVal;
                }
            }
            recalculateMatrices();
            document.getElementById('dispatchFeedbackMsg').innerHTML = `<span style="color:var(--state-safe);">RELEASED:</span> Returned resources to Available pool.`;
            addLogEntry(`${procName} released specified resources back to pool.`, 'info');
        });

        document.getElementById('clearLogBtn').addEventListener('click', () => {
            document.getElementById('logStream').innerHTML = '';
        });
    }

    // =========================================================================
    // 9. Startup & Initialization
    // =========================================================================
    window.addEventListener('DOMContentLoaded', () => {
        resizeCanvas();
        window.addEventListener('resize', resizeCanvas);

        loadTextbookSafePreset();
        initEventHandlers();
        startAnimationLoop();
    });

})();
