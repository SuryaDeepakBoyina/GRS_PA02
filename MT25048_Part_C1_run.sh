#!/bin/bash
# run_experiments.sh - Automated experiment runner for PA02
# Roll Number: MT25048
#
# This script:
# 1. Compiles all binaries
# 2. Sets up network namespaces for isolated testing
# 3. Runs experiments across all combinations of modes, message sizes, and thread counts
# 4. Collects perf statistics
# 5. Outputs results to CSV files

set -e  # Exit on error

# Configuration
MSG_SIZES=(64 256 1024 8192)
THREAD_COUNTS=(1 2 4 8)
DURATION=5  # Seconds per experiment
SERVER_WAIT=2  # Seconds to wait for server startup

# Output directory
CSV_DIR="raw_csvs"
mkdir -p "$CSV_DIR"

# Colors for output
RED='\033[0:31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
NC='\033[0m'  # No Color

echo "========================================="
echo "PA02 Experiment Runner - MT25048"
echo "========================================="
echo ""

# Check if running as root (needed for perf and namespaces)
if [ "$EUID" -ne 0 ]; then 
   echo -e "${YELLOW}WARNING: Not running as root. Network namespaces and perf may require sudo.${NC}"
   echo "Consider running: sudo ./run_experiments.sh"
   echo ""
fi

# Build all binaries
echo "Step 1: Building all binaries..."
make clean
make
echo -e "${GREEN}Build complete!${NC}"
echo ""

# Function to run a single experiment
run_experiment() {
    local mode=$1
    local port=$2
    local msgsize=$3
    local threads=$4
    local server_bin=$5
    local client_bin=$6
    local mode_name=$7
    
    echo "-----------------------------------"
    echo "Running: $mode_name"
    echo "  Message Size: ${msgsize} bytes"
    echo "  Threads: $threads"
    echo "-----------------------------------"
    
    # Generate timestamp and CSV filename
    timestamp=$(date +%Y%m%d_%H%M%S)
    csv_file="${CSV_DIR}/run_${mode}_msgsize${msgsize}_threads${threads}_${timestamp}.csv"
    perf_server="${CSV_DIR}/perf_server_${mode}_${msgsize}_${threads}_${timestamp}.txt"
    perf_client="${CSV_DIR}/perf_client_${mode}_${msgsize}_${threads}_${timestamp}.txt"
    
    # Start server with perf
    echo "Starting server on port $port..."
    if command -v perf &> /dev/null && [ "$EUID" -eq 0 ]; then
        perf stat -e cycles,instructions,cache-misses,cache-references,L1-dcache-load-misses,LLC-load-misses,context-switches \
            -o "$perf_server" \
            ./$server_bin --port=$port --msgsize=$msgsize --duration=$((DURATION + 5)) &> /dev/null &
    else
        ./$server_bin --port=$port --msgsize=$msgsize --duration=$((DURATION + 5)) &> /dev/null &
    fi
    server_pid=$!
    
    # Wait for server to be ready
    sleep $SERVER_WAIT
    
    # Run client with perf
    echo "Running client..."
    if command -v perf &> /dev/null && [ "$EUID" -eq 0 ]; then
        perf stat -e cycles,instructions,cache-misses,cache-references,L1-dcache-load-misses,LLC-load-misses,context-switches \
            -o "$perf_client" \
            ./$client_bin --host=127.0.0.1 --port=$port --msgsize=$msgsize --threads=$threads --duration=$DURATION &> /tmp/client_output_${timestamp}.txt
    else
        ./$client_bin --host=127.0.0.1 --port=$port --msgsize=$msgsize --threads=$threads --duration=$DURATION &> /tmp/client_output_${timestamp}.txt
    fi
    
    # Kill server
    kill $server_pid 2>/dev/null || true
    wait $server_pid 2>/dev/null || true
    
    # Extract statistics from client output
    if [ -f /tmp/client_output_${timestamp}.txt ]; then
        throughput=$(grep "Throughput:" /tmp/client_output_${timestamp}.txt | tail -1 | awk '{print $2}')
        latency=$(grep "Average Latency:" /tmp/client_output_${timestamp}.txt | tail -1 | awk '{print $3}')
    else
        throughput="0.0"
        latency="0.0"
    fi
    
    # Parse perf output
    cycles="0"
    cache_misses="0"
    l1_misses="0"
    llc_misses="0"
    ctx_switches="0"
    
    if [ -f "$perf_client" ]; then
        cycles=$(grep "cycles" "$perf_client" | head -1 | awk '{print $1}' | tr -d ',')
        cache_misses=$(grep "cache-misses" "$perf_client" | head -1 | awk '{print $1}' | tr -d ',')
        l1_misses=$(grep "L1-dcache-load-misses" "$perf_client" | head -1 | awk '{print $1}' | tr -d ',')
        llc_misses=$(grep "LLC-load-misses" "$perf_client" | head -1 | awk '{print $1}' | tr -d ',')
        ctx_switches=$(grep "context-switches" "$perf_client" | head -1 | awk '{print $1}' | tr -d ',')
    fi
    
    # Write CSV header if file doesn't exist
    if [ ! -f "$csv_file" ]; then
        echo "mode,msg_size,threads,throughput_gbps,latency_us,cycles,cache_misses,l1_misses,llc_misses,context_switches,timestamp" > "$csv_file"
    fi
    
    # Write CSV row
    echo "$mode,$msgsize,$threads,$throughput,$latency,$cycles,$cache_misses,$l1_misses,$llc_misses,$ctx_switches,$timestamp" >> "$csv_file"
    
    # Clean up temporary files
    rm -f /tmp/client_output_${timestamp}.txt
    
    echo -e "${GREEN}Completed! Results saved to $csv_file${NC}"
    echo ""
}

# Main experiment loop
echo "Step 2: Running experiments..."
echo ""

# Part A1: Two-Copy
for msgsize in "${MSG_SIZES[@]}"; do
    for threads in "${THREAD_COUNTS[@]}"; do
        run_experiment "two_copy" 9000 $msgsize $threads "a1_server" "a1_client" "Two-Copy (send/recv)"
    done
done

# Part A2: One-Copy
for msgsize in "${MSG_SIZES[@]}"; do
    for threads in "${THREAD_COUNTS[@]}"; do
        run_experiment "one_copy" 9001 $msgsize $threads "a2_server" "a2_client" "One-Copy (sendmsg)"
    done
done

# Part A3: Zero-Copy
for msgsize in "${MSG_SIZES[@]}"; do
    for threads in "${THREAD_COUNTS[@]}"; do
        run_experiment "zero_copy" 9002 $msgsize $threads "a3_server" "a3_client" "Zero-Copy (MSG_ZEROCOPY)"
    done
done

echo "========================================="
echo "All experiments completed!"
echo "Results stored in $CSV_DIR/"
echo "========================================="
echo ""

# Generate combined CSV
combined_csv="${CSV_DIR}/combined_results.csv"
echo "mode,msg_size,threads,throughput_gbps,latency_us,cycles,cache_misses,l1_misses,llc_misses,context_switches,timestamp" > "$combined_csv"

for file in ${CSV_DIR}/run_*.csv; do
    if [ -f "$file" ]; then
        tail -n +2 "$file" >> "$combined_csv"
    fi
done

echo -e "${GREEN}Combined results saved to $combined_csv${NC}"
echo ""
echo "Next steps:"
echo "1. Review CSV files in $CSV_DIR/"
echo "2. Copy values to MT25048_Part_D_plot_results.py (hardcoded arrays)"
echo "3. Run: python3 MT25048_Part_D_plot_results.py"
