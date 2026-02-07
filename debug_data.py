import pandas as pd
import os

csv_path = '/home/sboyina/Desktop/GRS_PA02/MT25048_PA02/raw_csvs/combined_results.csv'
df = pd.read_csv(csv_path)

MSG_SIZES = [64, 256, 1024, 8192]
data = {
    'cache_misses_two_copy': [],
    'cache_misses_one_copy': [],
    'cache_misses_zero_copy': [],
}

for msg_size in MSG_SIZES:
    for mode in ['two_copy', 'one_copy', 'zero_copy']:
        row = df[(df['mode'] == mode) & (df['msg_size'] == msg_size) & (df['threads'] == 4)]
        if not row.empty:
            val = row['cache_misses'].values[0]
            data[f'cache_misses_{mode}'].append(val)
            print(f"Mode: {mode}, MsgSize: {msg_size}, Cache Misses: {val}, Type: {type(val)}")

print("\nFinal Lists:")
for k, v in data.items():
    print(f"{k}: {v}")
