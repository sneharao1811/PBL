# IPC Mechanism Evaluation & Selection Justification

## 1. Executive Summary

During Phase 1 (Individual Research), each team member investigated, prototyped, and documented a primary IPC mechanism:
* **Waqqas**: POSIX Shared Memory (`shm_open`, `mmap`, `shm_unlink`)
* **Sneha**: Message Queues (`msgget`, `msgsnd`, `msgrcv`)
* **Siddanth**: Pipes & Named FIFOs (`pipe`, `mkfifo`)
* **Rafath**: Sockets, Signals & Semaphores (`socket`, `sigaction`, `sem_open`)

For the multi-process architecture (`src_v2`), the team selected **POSIX Message Queues (`mqueue.h`)** as the primary IPC transport. This document provides the engineering trade-off analysis and technical justification.

---

## 2. Quantitative & Qualitative Comparison Matrix

| Criteria | Shared Memory (`shm_open`) | Message Queues (`mqueue.h`) | Named FIFOs (`mkfifo`) | UNIX Domain Sockets (`AF_UNIX`) |
| :--- | :--- | :--- | :--- | :--- |
| **Message Boundaries** | No (Raw byte memory) | Yes (Preserves discrete message frames) | No (Stream-oriented) | Supported (Datagram mode) |
| **Synchronization** | Manual (Requires POSIX mutex/semaphores) | Built-in (Kernel handles queue locks) | Built-in (Blocking read/write semantics) | Built-in (Kernel buffer management) |
| **Complexity** | High (Memory layout, locking, race conditions) | Moderate (Clean queue API with attributes) | Low (Standard file descriptors) | Moderate to High (Socket lifecycle & bind/connect) |
| **Priority Support** | No | Built-in (Message priority field) | FIFO only | No |
| **Multiplexing** | Shared structure | Multiple receivers/senders per queue | Single writer/reader pairs | `select` / `epoll` |
| **Performance Overhead** | Near-zero (Direct RAM access) | Very Low (Kernel buffer copy) | Low (Kernel pipe buffer) | Low to Moderate (Socket stack) |

---

## 3. Justification for POSIX Message Queues

### Why Message Queues Won:
1. **Discrete Message Framing**: Unlike pipes and shared memory where bytes stream together without borders, POSIX message queues guarantee that each `mq_send()` delivers exactly one discrete `ipc_req_t` or `ipc_resp_t` packet to `mq_receive()`. No manual buffer length framing is required.
2. **Built-in Synchronization**: Shared memory requires auxiliary semaphores or mutexes in shared memory (`PTHREAD_PROCESS_SHARED`). A bug in lock release can deadlock the entire system. Message queues encapsulate atomic queueing within the kernel.
3. **Clean Process Decoupling**: Any process can read or write to `/pbl8_to_logger` without caring if the Logger process is currently busy. The kernel buffers up to `mq_maxmsg` messages automatically.
4. **POSIX Portability**: POSIX message queues (`<mqueue.h>`) provide deterministic file descriptor semantics and clean unlinking (`mq_unlink`) compared to legacy System V keys (`ftok`, `msgget`).

---

## 4. Why Other Alternatives Were Deferred

* **Shared Memory (`shm_open`)**: Ideal for ultra-large bulk payloads (e.g. video frames or huge matrices). For simulator commands (32-byte packets), the overhead of coordinating semaphores outweighs the nanosecond speed difference.
* **Named FIFOs (`mkfifo`)**: Pipes lack message boundaries. If two messages are sent rapidly, `read()` can read both simultaneously or fragment them, requiring custom framing logic.
* **UNIX Domain Sockets (`AF_UNIX`)**: An excellent secondary choice, but involves socket binding, filesystem inode management for socket files, and more boilerplate than message queues for simple RPC/logging patterns.
