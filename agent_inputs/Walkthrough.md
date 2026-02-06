Walkthrough (what agent must do, step-by-step)

Initialize repo

mkdir GRS_PA02 && cd GRS_PA02

git init (or push to Github and make public)

Create README.md, Makefile (top-line comment with roll no).

Baseline server & client

Implement simple TCP server: accept, create thread per client, each thread repeatedly sends fixed-size message struct with 8 heap-allocated string fields (~use malloc for each field per requirement).

Implement client: open connection(s), spawn T threads that send/recv continuously for duration.

Add command-line parsing for parameters.

Verify with small msgsize=256, threads=1.

One-copy (sendmsg)

Replace send() with sendmsg() and use iovec pointing to preallocated buffers.

Use mmap or posix_memalign for aligned buffers (makes DMA-friendly).

Document which copy is removed in code comments and README.

Zero-copy (MSG_ZEROCOPY)

Add MSG_ZEROCOPY flag with proper feature checks (kernel version).

Implement completion handling: use recvmsg() on a SO_ZEROCOPY completion socket or read MSG_ZEROCOPY notifications from errno == EBUSY and msg_controllen as required. (Document kernel behavior and limitations.)

Carefully pin/unpin pages if necessary; handle errors and fallback.

Perf instrumentation

Use wrapper to run perf stat -e cycles,cache-misses,cache-references,context-switches -o perf_raw.txt ./server ...

On client side measure throughput and latency (timestamp-based RTT or per message latency).

Automated experiments

Script loops: for msg in 64 256 1024 8192; do for th in 1 2 4 8; do run server & wait; run client; collect perf; kill server; done; done

Each run writes CSV filename containing mode-msg-threads-timestamp.csv.

Postprocessing & plotting

Parse perf outputs to CSV

Use plot_results.py to read CSVs and create plots, but adhere to PA02: hardcode arrays in the python script for plotting (the script should copy numbers from CSV into array constants—include instructions for the agent to populate these arrays after the run).

Report generation

Create report skeleton with required sections (methodology, raw CSVs, plots embedded in the report PDF, AI usage section, repo URL).

Add screenshots of perf outputs and run logs.

Final checks and packaging

Verify naming conventions.

Zip only required files into <roll_num>_PA02.zip.