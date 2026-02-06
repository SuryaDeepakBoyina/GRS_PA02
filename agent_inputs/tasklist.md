Task list (atomic actions for the agent)

Fetch spec & repo prep

Load PA02 spec (

PA02_ networkIO

) and create repo folder GRS_PA02/<roll_num>_PA02.

Create .gitignore, Makefile, README.md (with roll no. comment in top).

Create code skeletons

MT25xxx_Part_A1_Server.c

MT25xxx_Part_A1_Client.c

MT25xxx_Part_A2_Server.c / Client.c (sendmsg / pre-registered buffer)

MT25xxx_Part_A3_Server.c / Client.c (MSG_ZEROCOPY)

common.h, common.c (message struct alloc/free helpers)

Implement networking primitives

Implement baseline blocking send()/recv() server & client (thread-per-client).

Implement sendmsg() variant with scatter/gather / pre-registered buffers.

Implement MSG_ZEROCOPY variant and completion handling (use sendmsg() flags & SOL_SOCKET notifications).

Add multithreading & parameterization

Runtime flags: --mode=[two|one|zero] --msgsize=NN --threads=T --duration=S --port=P

Use pthreads for thread-per-client server; client spawns T sender threads.

Create experiment runner

run_experiments.sh:

Compile all binaries

Loop over message sizes and thread counts

Run server in one network namespace (use ip netns), launch client(s) in another

Capture stdout/stderr and perf output into CSV-named files

Perf collection

Use perf stat (and perf record if needed): collect cycles, cache-misses, context-switches, L1/L2/LLC misses.

Save raw perf outputs and parse into CSV rows (script: parse_perf.py).

Data storage

Standardize CSV format: mode,msg_size,threads,throughput_gbps,latency_us,cycles,cache_misses,l1_misses,llc_misses,context_switches,timestamp

Plotting

plot_results.py (matplotlib, hardcoded arrays for each plot per assignment rules).

Generate: throughput vs msg size, latency vs thread count, cache misses vs msg size, cycles per byte.

Report & README

Create MT25xxx_PA02_Report.pdf contents (templates for sections: methodology, results, analysis, AI usage).

Add AI usage log file AI_prompts_used.txt.

Sanity & compliance checks

Ensure filenames match naming convention.

Ensure no binary files in zip.

Ensure README & Makefile contain roll number comment.

Packaging

Create zip <roll_num>_PA02.zip containing only allowed filesTask list (atomic actions for the agent)

Fetch spec & repo prep

Load PA02 spec (

PA02_ networkIO

) and create repo folder GRS_PA02/<roll_num>_PA02.

Create .gitignore, Makefile, README.md (with roll no. comment in top).

Create code skeletons

MT25xxx_Part_A1_Server.c

MT25xxx_Part_A1_Client.c

MT25xxx_Part_A2_Server.c / Client.c (sendmsg / pre-registered buffer)

MT25xxx_Part_A3_Server.c / Client.c (MSG_ZEROCOPY)

common.h, common.c (message struct alloc/free helpers)

Implement networking primitives

Implement baseline blocking send()/recv() server & client (thread-per-client).

Implement sendmsg() variant with scatter/gather / pre-registered buffers.

Implement MSG_ZEROCOPY variant and completion handling (use sendmsg() flags & SOL_SOCKET notifications).

Add multithreading & parameterization

Runtime flags: --mode=[two|one|zero] --msgsize=NN --threads=T --duration=S --port=P

Use pthreads for thread-per-client server; client spawns T sender threads.

Create experiment runner

run_experiments.sh:

Compile all binaries

Loop over message sizes and thread counts

Run server in one network namespace (use ip netns), launch client(s) in another

Capture stdout/stderr and perf output into CSV-named files

Perf collection

Use perf stat (and perf record if needed): collect cycles, cache-misses, context-switches, L1/L2/LLC misses.

Save raw perf outputs and parse into CSV rows (script: parse_perf.py).

Data storage

Standardize CSV format: mode,msg_size,threads,throughput_gbps,latency_us,cycles,cache_misses,l1_misses,llc_misses,context_switches,timestamp

Plotting

plot_results.py (matplotlib, hardcoded arrays for each plot per assignment rules).

Generate: throughput vs msg size, latency vs thread count, cache misses vs msg size, cycles per byte.

Report & README

Create MT25xxx_PA02_Report.pdf contents (templates for sections: methodology, results, analysis, AI usage).

Add AI usage log file AI_prompts_used.txt.

Sanity & compliance checks

Ensure filenames match naming convention.

Ensure no binary files in zip.

Ensure README & Makefile contain roll number comment.

Packaging

Create zip <roll_num>_PA02.zip containing only allowed files