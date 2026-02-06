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
System: Linux kernel 5.15+
CPU: Intel/AMD x64
RAM: 16GB
Date: 2026-02-05
"""

# ============================================================================
# HARDCODED DATA ARRAYS - MANUALLY UPDATE AFTER EXPERIMENTS
# ============================================================================

# Message sizes tested (in bytes)
MSG_SIZES = [64, 256, 1024, 8192]

# Thread counts tested
THREAD_COUNTS = [1, 2, 4, 8]

# --- Throughput Data (Gbps) ---
# TODO: After running experiments, copy values from combined_results.csv here

# Throughput for each mode, varying message size (threads=4 as baseline)
throughput_two_copy_by_msgsize = [0.5, 1.2, 2.3, 3.1]      # Example values - REPLACE
throughput_one_copy_by_msgsize = [0.7, 1.5, 2.8, 3.8]      # Example values - REPLACE
throughput_zero_copy_by_msgsize = [0.6, 1.3, 3.5, 5.2]     # Example values - REPLACE

# --- Latency Data (µs) ---
# Average latency for each mode, varying thread count (msgsize=1024 as baseline)
latency_two_copy_by_threads = [45.2, 52.3, 78.5, 125.6]    # Example values - REPLACE
latency_one_copy_by_threads = [38.1, 45.7, 65.2, 98.3]     # Example values - REPLACE
latency_zero_copy_by_threads = [42.5, 48.9, 70.1, 110.4]   # Example values - REPLACE

# --- Cache Miss Data (count) ---
# Cache misses for each mode, varying message size (threads=4)
cache_misses_two_copy = [1200, 3500, 8900, 25000]          # Example values - REPLACE
cache_misses_one_copy = [1000, 3000, 7500, 20000]          # Example values - REPLACE
cache_misses_zero_copy = [950, 2800, 7200, 18500]          # Example values - REPLACE

# L1 cache misses
l1_misses_two_copy = [800, 2300, 6000, 18000]              # Example values - REPLACE
l1_misses_one_copy = [650, 1900, 5000, 14000]              # Example values - REPLACE
l1_misses_zero_copy = [600, 1800, 4800, 13500]             # Example values - REPLACE

# LLC misses
llc_misses_two_copy = [350, 1100, 2800, 8500]              # Example values - REPLACE
llc_misses_one_copy = [300, 950, 2200, 6500]               # Example values - REPLACE
llc_misses_zero_copy = [280, 900, 2000, 6000]              # Example values - REPLACE

# --- CPU Cycles Data ---
# Total cycles for each mode, varying message size
cycles_two_copy = [50000, 180000, 650000, 2100000]         # Example values - REPLACE
cycles_one_copy = [45000, 160000, 580000, 1850000]         # Example values - REPLACE
cycles_zero_copy = [48000, 170000, 550000, 1650000]        # Example values - REPLACE

# Total bytes transferred (for cycles per byte calculation)
# Calculated as: msg_size * messages_sent
bytes_transferred = [64000, 256000, 1024000, 8192000]      # Example values - REPLACE

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
    
    # Total cache misses
    axes[0].plot(MSG_SIZES, cache_misses_two_copy, 'o-', label='Two-Copy', color='#e74c3c')
    axes[0].plot(MSG_SIZES, cache_misses_one_copy, 's-', label='One-Copy', color='#3498db')
    axes[0].plot(MSG_SIZES, cache_misses_zero_copy, '^-', label='Zero-Copy', color='#2ecc71')
    axes[0].set_xlabel('Message Size (bytes)')
    axes[0].set_ylabel('Total Cache Misses')
    axes[0].set_title('Total Cache Misses')
    axes[0].set_xscale('log', base=2)
    axes[0].grid(True, alpha=0.3)
    axes[0].legend()
    
    # L1 cache misses
    axes[1].plot(MSG_SIZES, l1_misses_two_copy, 'o-', label='Two-Copy', color='#e74c3c')
    axes[1].plot(MSG_SIZES, l1_misses_one_copy, 's-', label='One-Copy', color='#3498db')
    axes[1].plot(MSG_SIZES, l1_misses_zero_copy, '^-', label='Zero-Copy', color='#2ecc71')
    axes[1].set_xlabel('Message Size (bytes)')
    axes[1].set_ylabel('L1 Cache Misses')
    axes[1].set_title('L1 Cache Misses')
    axes[1].set_xscale('log', base=2)
    axes[1].grid(True, alpha=0.3)
    axes[1].legend()
    
    # LLC misses
    axes[2].plot(MSG_SIZES, llc_misses_two_copy, 'o-', label='Two-Copy', color='#e74c3c')
    axes[2].plot(MSG_SIZES, llc_misses_one_copy, 's-', label='One-Copy', color='#3498db')
    axes[2].plot(MSG_SIZES, llc_misses_zero_copy, '^-', label='Zero-Copy', color='#2ecc71')
    axes[2].set_xlabel('Message Size (bytes)')
    axes[2].set_ylabel('LLC Misses')
    axes[2].set_title('LLC Misses')
    axes[2].set_xscale('log', base=2)
    axes[2].grid(True, alpha=0.3)
    axes[2].legend()
    
    plt.suptitle('Cache Misses vs Message Size (4 threads)')
    plt.tight_layout()
    plt.savefig('MT25048_Plot_CacheMisses_vs_MsgSize.pdf')
    print("Saved: MT25048_Plot_CacheMisses_vs_MsgSize.pdf")
    plt.close()

def plot_cycles_per_byte():
    """Plot 4: CPU Cycles per Byte Transferred."""
    # Calculate cycles per byte
    cycles_per_byte_two = [cycles_two_copy[i] / bytes_transferred[i] for i in range(len(MSG_SIZES))]
    cycles_per_byte_one = [cycles_one_copy[i] / bytes_transferred[i] for i in range(len(MSG_SIZES))]
    cycles_per_byte_zero = [cycles_zero_copy[i] / bytes_transferred[i] for i in range(len(MSG_SIZES))]
    
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
    print("IMPORTANT: Ensure data arrays have been updated with actual")
    print("           experimental values from CSV files!")
    print()
    
    # Check if using example data
    if throughput_two_copy_by_msgsize[0] == 0.5:
        print("WARNING: Still using example data!")
        print("         Update hardcoded arrays before final submission.")
        print()
        response = input("Continue with example data? (y/n): ")
        if response.lower() != 'y':
            print("Aborted. Please update data arrays first.")
            return 1
    
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
