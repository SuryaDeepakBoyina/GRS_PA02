Implementation plan (files, code snippets, and checks)
Repo layout (exact)
GRS_PA02/
  MT25xxx_Part_A1_Server.c
  MT25xxx_Part_A1_Client.c
  MT25xxx_Part_A2_Server.c
  MT25xxx_Part_A2_Client.c
  MT25xxx_Part_A3_Server.c
  MT25xxx_Part_A3_Client.c
  common.h
  common.c
  Makefile
  run_experiments.sh
  parse_perf.py
  plot_results.py
  README.md
  AI_prompts_used.txt
  raw_csvs/    <-- all raw run CSVs
  report/      <-- report files & screenshots (report.pdf)

Example Makefile (top comment with roll no)
# MT25xxx - Makefile
CC = gcc
CFLAGS = -O2 -pthread -g
all: a1_server a1_client a2_server a2_client a3_server a3_client

a1_server: MT25xxx_Part_A1_Server.c common.c
	$(CC) $(CFLAGS) -o $@ MT25xxx_Part_A1_Server.c common.c

# ... similarly for others

clean:
	rm -f a1_server a1_client a2_server a2_client a3_server a3_client

Key code pointers for agent

Use setsockopt() to configure send/recv buffer sizes for controlled experiments.

Use clock_gettime(CLOCK_MONOTONIC) for latency micro-benchmarks.

Use robust signal handling for clean shutdown (SIGINT).

Ensure heap-allocated string fields are malloced per message as spec requires.

Perf command examples

Basic:

perf stat -e cycles,cache-misses,cache-references,context-switches -o perf_raw.txt ./a1_server --mode=two --port=9000


For more counters:

perf stat -e cycles,instructions,cache-misses,cache-references,L1-dcache-load-misses,LLC-load-misses -o perf_raw.txt ./a1_client ...

CSV format (single-run)

Columns:

mode,msg_size,threads,throughput_gbps,median_latency_us,cycles,cache_misses,l1_misses,llc_misses,context_switches,run_id


Agent must append one row per run.

Plotting (must follow PA02 requirement)

plot_results.py must define arrays like:

msg_sizes = [64, 256, 1024, 8192]
throughputs_two = [1.2, 3.4, 4.1, 2.8]  # agent must replace after runs
# plot using matplotlib with labels, legend, system config text


Produce plots as PDF and also embed into final report.