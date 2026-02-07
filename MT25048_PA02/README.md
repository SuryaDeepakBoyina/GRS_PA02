# MT25048 - PA02: Network I/O Analysis using perf
**Roll Number:** MT25048  
**Course:** Graduate Systems (CSE638)  
**Assignment:** PA02 - Analysis of Network I/O primitives  

## Overview

This project implements and analyzes three variants of network I/O:
- **Two-Copy** (baseline): Standard `send()`/`recv()` socket communication
- **One-Copy**: Optimized using `sendmsg()` with pre-registered buffers
- **Zero-Copy**: Using `sendmsg()` with `MSG_ZEROCOPY` flag

Each variant is profiled using `perf` to analyze CPU cycles, cache behavior, and context switches.

## Build Instructions

```bash
cd MT25048_PA02
make clean
make
```

This will generate 6 binaries:
- `a1_server`, `a1_client` (two-copy)
- `a2_server`, `a2_client` (one-copy)
- `a3_server`, `a3_client` (zero-copy)

## Running Experiments

### Automated (Recommended)
```bash
sudo ./MT25048_Part_C1_run.sh
```



### Manual Testing

**Server (in one terminal):**
```bash
./a1_server --port 9000 --msgsize 1024 --duration 5
```

**Client (in another terminal):**
```bash
./a1_client --host 127.0.0.1 --port 9000 --threads 4 --msgsize 1024 --duration 5
```



## Generating Plots

```bash
python3 MT25048_Part_D_plot_results.py
```



## Project Structure

```
MT25048_PA02/
├── MT25048_Part_A_common.h               # Message structure and utilities
├── MT25048_Part_A_common.c               # Implementation of common utilities
├── MT25048_Part_A1_Server.c              # Two-copy server
├── MT25048_Part_A1_Client.c              # Two-copy client
├── MT25048_Part_A2_Server.c              # One-copy server
├── MT25048_Part_A2_Client.c              # One-copy client
├── MT25048_Part_A3_Server.c              # Zero-copy server
├── MT25048_Part_A3_Client.c              # Zero-copy client
├── MT25048_Part_C1_run.sh                 # Automated experiment runner
├── MT25048_Part_D_plot_results.py        # Generates plots from hardcoded data
├── Makefile                              # Build system
├── README.md                             # This file
├── MT25048_Part_C2_CSV.csv               # CSV file for Part C2
```



## Implementation Details

### Two-Copy (A1)
Uses standard `send()`/`recv()`. Data is copied from user buffer to kernel socket buffer, then from kernel to NIC.

### One-Copy (A2)
Uses `sendmsg()` with `iovec` structures. Pre-aligned buffers allow kernel to read directly from user space, eliminating one copy.

### Zero-Copy (A3)
Uses `sendmsg()` with `MSG_ZEROCOPY` flag. Kernel pins user pages and DMA transfers directly to NIC. Requires completion notification handling.

## Metrics Collected

- **Throughput** (Gbps): Application-level measurement
- **Latency** (µs): Round-trip time measurement
- **CPU Cycles**: Via `perf stat`
- **Cache Misses** (L1, LLC): Via `perf stat`
- **Context Switches**: Via `perf stat`


