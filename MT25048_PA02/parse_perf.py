#!/usr/bin/env python3
"""
parse_perf.py - Parse perf output files to CSV format
Roll Number: MT25048

This script parses perf stat output and combines it with application metrics.
"""

import sys
import re
import glob
import os

def parse_perf_file(filepath):
    """Parse a perf stat output file and extract metrics."""
    metrics = {
        'cycles': 0,
        'instructions': 0,
        'cache_misses': 0,
        'cache_references': 0,
        'l1_misses': 0,
        'llc_misses': 0,
        'context_switches': 0
    }
    
    try:
        with open(filepath, 'r') as f:
            content = f.read()
            
            # Parse each metric
            patterns = {
                'cycles': r'([\d,]+)\s+cycles',
                'instructions': r'([\d,]+)\s+instructions',
                'cache_misses': r'([\d,]+)\s+cache-misses',
                'cache_references': r'([\d,]+)\s+cache-references',
                'l1_misses': r'([\d,]+)\s+L1-dcache-load-misses',
                'llc_misses': r'([\d,]+)\s+LLC-load-misses',
                'context_switches': r'([\d,]+)\s+context-switches'
            }
            
            for metric, pattern in patterns.items():
                match = re.search(pattern, content)
                if match:
                    value_str = match.group(1).replace(',', '')
                    metrics[metric] = int(value_str)
    
    except Exception as e:
        print(f"Error parsing {filepath}: {e}", file=sys.stderr)
    
    return metrics

def main():
    """Main function to parse all perf files in raw_csvs directory."""
    csv_dir = "raw_csvs"
    
    if not os.path.exists(csv_dir):
        print(f"Directory {csv_dir} not found!", file=sys.stderr)
        return 1
    
    perf_files = glob.glob(os.path.join(csv_dir, "perf_*.txt"))
    
    print(f"Found {len(perf_files)} perf output files")
    
    for perf_file in perf_files:
        print(f"Parsing {perf_file}...")
        metrics = parse_perf_file(perf_file)
        
        print(f"  Cycles: {metrics['cycles']:,}")
        print(f"  Cache Misses: {metrics['cache_misses']:,}")
        print(f"  L1 Misses: {metrics['l1_misses']:,}")
        print(f"  LLC Misses: {metrics['llc_misses']:,}")
        print(f"  Context Switches: {metrics['context_switches']:,}")
        print()
    
    print("Perf parsing complete!")
    print("\nNote: The run_experiments.sh script already integrates perf metrics into CSV files.")
    print("This script is provided for additional manual parsing if needed.")
    
    return 0

if __name__ == "__main__":
    sys.exit(main())
