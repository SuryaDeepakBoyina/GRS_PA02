#!/usr/bin/env python3
"""
MT25048_Part_D_plot_results.py - Generate plots for PA02 Network I/O Analysis
Roll Number: MT25048

CRITICAL REQUIREMENT:
All data MUST be hardcoded as arrays below. Do NOT read from CSV files.
After running experiments, manually copy values from CSV files into the arrays below.

This script generates 4 required plots:
1. Throughput vs Message Size
2. Latency vs Thread Count
3. Cache Misses vs Message Size
4. CPU Cycles per Byte Transferred
"""

import matplotlib.pyplot as plt
import numpy as np
import sys

# System configuration (update with your actual system specs)
SYSTEM_CONFIG = """
System: Fedora Linux 42 (KDE Plasma Desktop Edition)
CPU: Intel x86
RAM: 8GB
Date: 2026-02-07
"""

# ============================================================================
# HARDCODED EXPERIMENTAL DATA
# ============================================================================
# Data extracted from raw_csvs/combined_results.csv (timestamp: 20260207_01xxxx)
# After client/server role fix: Client SENDS, Server RECEIVES
# Baseline: threads=4 for throughput/cache plots, msgsize=1024 for latency plots

# Message sizes tested (in bytes)
MSG_SIZES = [64, 256, 1024, 8192]

# Thread counts tested
THREAD_COUNTS = [1, 2, 4, 8]

# Duration used in experiments (from run_experiments.sh)
DURATION = 5  # seconds

# ============================================================================
# THROUGHPUT DATA (Gbps) - threads=4, varying message size
# Format: [64 bytes, 256 bytes, 1024 bytes, 8192 bytes]
# ============================================================================
throughput_two_copy_by_msgsize = [0.34, 1.615, 8.169, 21.402]
throughput_one_copy_by_msgsize = [0.276, 1.413, 4.897, 38.681]
throughput_zero_copy_by_msgsize = [0.187, 0.941, 2.443, 30.998]

# ============================================================================
# LATENCY DATA (µs) - msgsize=1024, varying threads
# Format: [1 thread, 2 threads, 4 threads, 8 threads]
# ============================================================================
latency_two_copy_by_threads = [5.01, 2.26, 3.97, 5.70]
latency_one_copy_by_threads = [2.92, 3.75, 6.13, 6.40]
latency_zero_copy_by_threads = [3.88, 5.19, 12.04, 17.10]

# ============================================================================
# CACHE MISS DATA - threads=4, varying message size
# Format: [64 bytes, 256 bytes, 1024 bytes, 8192 bytes]
# ============================================================================

# cache_misses (perf: cache-misses - misses that went to main memory)
cache_misses_two_copy = [3206406, 4864440, 6166287, 25286368]
cache_misses_one_copy = [2982470, 4322291, 16282255, 249654615]
cache_misses_zero_copy = [4435749, 3546400, 6382138, 25883728]

# L1 cache misses (perf: L1-dcache-load-misses)
l1_misses_two_copy = [493340606, 258619045, 270940225, 720812096]
l1_misses_one_copy = [604435799, 590554636, 663760945, 1208667898]
l1_misses_zero_copy = [747425854, 764573458, 803294755, 738661137]

# LLC cache misses (perf: LLC-load-misses)
llc_misses_two_copy = [74978, 100548, 83445, 181741]
llc_misses_one_copy = [60839, 75496, 157943, 1213294]
llc_misses_zero_copy = [28294, 47061, 141573, 593154]

# ============================================================================
# CPU CYCLES DATA - threads=4, varying message size
# Format: [64 bytes, 256 bytes, 1024 bytes, 8192 bytes]
# ============================================================================
cycles_two_copy = [33001693645, 25372980499, 18010295414, 15586843165]
cycles_one_copy = [39438779804, 44511469539, 39466498693, 28048004914]
cycles_zero_copy = [40543575711, 44227911279, 42751881102, 30188950831]

# ============================================================================
# CALCULATED DATA: Bytes transferred = (throughput_gbps * DURATION * 1e9) / 8
# ============================================================================
bytes_two_copy = [(t * DURATION * 1e9) / 8 for t in throughput_two_copy_by_msgsize]
bytes_one_copy = [(t * DURATION * 1e9) / 8 for t in throughput_one_copy_by_msgsize]
bytes_zero_copy = [(t * DURATION * 1e9) / 8 for t in throughput_zero_copy_by_msgsize]

# ============================================================================
# PLOTTING FUNCTIONS
# ============================================================================

def setup_plot_style():
    """Configure matplotlib style for professional plots."""
    plt.rcParams['figure.figsize'] = (10, 6)
    plt.rcParams['font.size'] = 11
    plt.rcParams['axes.labelsize'] = 12
    plt.rcParams['axes.titlesize'] = 14
    plt.rcParams['legend.fontsize'] = 10
    plt.rcParams['lines.linewidth'] = 2
    plt.rcParams['lines.markersize'] = 8

def plot_throughput_vs_msgsize():
    """Plot 1: Throughput vs Message Size."""
    plt.figure()
    
    plt.plot(MSG_SIZES, throughput_two_copy_by_msgsize, 'o-', label='Two-Copy (send/recv)', color='#e74c3c')
    plt.plot(MSG_SIZES, throughput_one_copy_by_msgsize, 's-', label='One-Copy (sendmsg)', color='#3498db')
    plt.plot(MSG_SIZES, throughput_zero_copy_by_msgsize, '^-', label='Zero-Copy (MSG_ZEROCOPY)', color='#2ecc71')
    
    plt.xlabel('Message Size (bytes)')
    plt.ylabel('Throughput (Gbps)')
    plt.title('Throughput vs Message Size (4 threads)')
    plt.xscale('log', base=2)
    plt.grid(True, alpha=0.3)
    plt.legend()
    
    # Add system config annotation
    plt.text(0.02, 0.98, SYSTEM_CONFIG.strip(), 
             transform=plt.gca().transAxes, 
             fontsize=8, 
             verticalalignment='top',
             bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.5))
    
    plt.tight_layout()
    plt.savefig('MT25048_Plot_Throughput_vs_MsgSize.pdf')
    print("Saved: MT25048_Plot_Throughput_vs_MsgSize.pdf")
    plt.close()

def plot_latency_vs_threads():
    """Plot 2: Latency vs Thread Count."""
    plt.figure()
    
    plt.plot(THREAD_COUNTS, latency_two_copy_by_threads, 'o-', label='Two-Copy', color='#e74c3c')
    plt.plot(THREAD_COUNTS, latency_one_copy_by_threads, 's-', label='One-Copy', color='#3498db')
    plt.plot(THREAD_COUNTS, latency_zero_copy_by_threads, '^-', label='Zero-Copy', color='#2ecc71')
    
    plt.xlabel('Thread Count')
    plt.ylabel('Average Latency (µs)')
    plt.title('Latency vs Thread Count (1024-byte messages)')
    plt.grid(True, alpha=0.3)
    plt.legend()
    plt.xticks(THREAD_COUNTS)
    
    plt.text(0.02, 0.98, SYSTEM_CONFIG.strip(), 
             transform=plt.gca().transAxes, 
             fontsize=8, 
             verticalalignment='top',
             bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.5))
    
    plt.tight_layout()
    plt.savefig('MT25048_Plot_Latency_vs_Threads.pdf')
    print("Saved: MT25048_Plot_Latency_vs_Threads.pdf")
    plt.close()

def plot_cache_misses_vs_msgsize():
    """Plot 3: Cache Misses vs Message Size."""
    fig, axes = plt.subplots(1, 3, figsize=(15, 5))
    
    # Plot 1: L1 Data Cache Misses
    axes[0].plot(MSG_SIZES, [v/1e6 for v in l1_misses_two_copy], 'o-', label='Two-Copy', color='#e74c3c')
    axes[0].plot(MSG_SIZES, [v/1e6 for v in l1_misses_one_copy], 's-', label='One-Copy', color='#3498db')
    axes[0].plot(MSG_SIZES, [v/1e6 for v in l1_misses_zero_copy], '^-', label='Zero-Copy', color='#2ecc71')
    axes[0].set_xlabel('Message Size (bytes)')
    axes[0].set_ylabel('Misses (Millions)')
    axes[0].set_title('L1 Data Cache Misses')
    axes[0].set_xscale('log', base=2)
    axes[0].grid(True, alpha=0.3)
    axes[0].legend()
    
    # Plot 2: Cache misses to memory (perf cache-misses)
    axes[1].plot(MSG_SIZES, [v/1e6 for v in cache_misses_two_copy], 'o-', label='Two-Copy', color='#e74c3c')
    axes[1].plot(MSG_SIZES, [v/1e6 for v in cache_misses_one_copy], 's-', label='One-Copy', color='#3498db')
    axes[1].plot(MSG_SIZES, [v/1e6 for v in cache_misses_zero_copy], '^-', label='Zero-Copy', color='#2ecc71')
    axes[1].set_xlabel('Message Size (bytes)')
    axes[1].set_ylabel('Misses (Millions)')
    axes[1].set_title('Cache-to-Memory Misses')
    axes[1].set_xscale('log', base=2)
    axes[1].grid(True, alpha=0.3)
    axes[1].legend()
    
    # Plot 3: LLC Load Misses
    axes[2].plot(MSG_SIZES, [v/1e6 for v in llc_misses_two_copy], 'o-', label='Two-Copy', color='#e74c3c')
    axes[2].plot(MSG_SIZES, [v/1e6 for v in llc_misses_one_copy], 's-', label='One-Copy', color='#3498db')
    axes[2].plot(MSG_SIZES, [v/1e6 for v in llc_misses_zero_copy], '^-', label='Zero-Copy', color='#2ecc71')
    axes[2].set_xlabel('Message Size (bytes)')
    axes[2].set_ylabel('Misses (Millions)')
    axes[2].set_title('LLC Load Misses')
    axes[2].set_xscale('log', base=2)
    axes[2].grid(True, alpha=0.3)
    axes[2].legend()
    
    plt.suptitle('Cache Misses vs Message Size (4 threads)')
    
    # Add system config annotation (centered at the bottom)
    fig.text(0.5, 0.02, SYSTEM_CONFIG.strip().replace('\n', ' | '), 
             ha='center', fontsize=9, 
             bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.5))
    
    plt.tight_layout(rect=[0, 0.05, 1, 0.95]) # Adjust layout to make room for suptitle and config
    plt.savefig('MT25048_Plot_CacheMisses_vs_MsgSize.pdf')
    print("Saved: MT25048_Plot_CacheMisses_vs_MsgSize.pdf")
    plt.close()

def plot_cycles_per_byte():
    """Plot 4: CPU Cycles per Byte Transferred."""
    # Calculate cycles per byte
    cycles_per_byte_two = [cycles_two_copy[i] / bytes_two_copy[i] for i in range(len(MSG_SIZES))]
    cycles_per_byte_one = [cycles_one_copy[i] / bytes_one_copy[i] for i in range(len(MSG_SIZES))]
    cycles_per_byte_zero = [cycles_zero_copy[i] / bytes_zero_copy[i] for i in range(len(MSG_SIZES))]
    
    plt.figure()
    
    plt.plot(MSG_SIZES, cycles_per_byte_two, 'o-', label='Two-Copy', color='#e74c3c')
    plt.plot(MSG_SIZES, cycles_per_byte_one, 's-', label='One-Copy', color='#3498db')
    plt.plot(MSG_SIZES, cycles_per_byte_zero, '^-', label='Zero-Copy', color='#2ecc71')
    
    plt.xlabel('Message Size (bytes)')
    plt.ylabel('CPU Cycles per Byte')
    plt.title('CPU Cycles per Byte Transferred (4 threads)')
    plt.xscale('log', base=2)
    plt.grid(True, alpha=0.3)
    plt.legend()
    
    plt.text(0.02, 0.98, SYSTEM_CONFIG.strip(), 
             transform=plt.gca().transAxes, 
             fontsize=8, 
             verticalalignment='top',
             bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.5))
    
    plt.tight_layout()
    plt.savefig('MT25048_Plot_CyclesPerByte.pdf')
    print("Saved: MT25048_Plot_CyclesPerByte.pdf")
    plt.close()

# ============================================================================
# MAIN FUNCTION
# ============================================================================

def main():
    """Generate all required plots."""
    print("="*60)
    print("MT25048 - PA02 Plot Generator")
    print("="*60)
    print()
    print("NOTE: All data is HARDCODED in this script as per assignment requirements.")
    print("      Data was extracted from raw_csvs/combined_results.csv")
    print("      Experiment date: 2026-02-07 (after client/server role fix)")
    print()
    
    setup_plot_style()
    
    print("Generating plots...")
    plot_throughput_vs_msgsize()
    plot_latency_vs_threads()
    plot_cache_misses_vs_msgsize()
    plot_cycles_per_byte()
    
    print()
    print("="*60)
    print("All plots generated successfully!")
    print("="*60)
    print()
    print("Files created:")
    print("  - MT25048_Plot_Throughput_vs_MsgSize.pdf")
    print("  - MT25048_Plot_Latency_vs_Threads.pdf")
    print("  - MT25048_Plot_CacheMisses_vs_MsgSize.pdf")
    print("  - MT25048_Plot_CyclesPerByte.pdf")
    print()
    print("Next steps:")
    print("  1. Embed these plots in your report PDF")
    print("  2. Do NOT include plot files in the zip submission")
    print("  3. During demo, run this script to regenerate plots")
    
    return 0

if __name__ == "__main__":
    sys.exit(main())
