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
./run_experiments.sh
```

This script:
1. Compiles all binaries
2. Sets up network namespaces
3. Runs all combinations of message sizes (64, 256, 1024, 8192 bytes) and thread counts (1, 2, 4, 8)
4. Collects `perf` metrics
5. Outputs results to `raw_csvs/`

### Manual Testing

**Server (in one terminal):**
```bash
./a1_server --port 9000 --msgsize 1024 --duration 30
```

**Client (in another terminal):**
```bash
./a1_client --host 127.0.0.1 --port 9000 --threads 4 --msgsize 1024 --duration 30
```

Replace `a1_*` with `a2_*` or `a3_*` for other variants.

## Generating Plots

```bash
python3 MT25048_Part_D_plot_results.py
```

**Note:** Values are hardcoded in the Python script as per assignment requirements.

## Project Structure

```
MT25048_PA02/
├── common.h                              # Message structure and utilities
├── common.c                              # Implementation of common utilities
├── MT25048_Part_A1_Server.c              # Two-copy server
├── MT25048_Part_A1_Client.c              # Two-copy client
├── MT25048_Part_A2_Server.c              # One-copy server
├── MT25048_Part_A2_Client.c              # One-copy client
├── MT25048_Part_A3_Server.c              # Zero-copy server
├── MT25048_Part_A3_Client.c              # Zero-copy client
├── run_experiments.sh                    # Automated experiment runner
├── parse_perf.py                         # Parses perf output to CSV
├── MT25048_Part_D_plot_results.py        # Generates plots from hardcoded data
├── Makefile                              # Build system
├── README.md                             # This file
├── AI_prompts_used.txt                   # AI usage documentation
├── raw_csvs/                             # Experimental results
└── report/                               # Report and screenshots
```

## System Requirements

- Linux kernel 4.14+ (for MSG_ZEROCOPY support)
- GCC with pthread support
- Python 3.x with matplotlib
- perf tool
- Root/sudo access for network namespaces and perf

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

## References

See `AI_prompts_used.txt` for detailed AI usage documentation.

## GitHub Repository

https://github.com/[username]/GRS_PA02
