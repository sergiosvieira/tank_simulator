#!/usr/bin/env python3
"""
Plot comparison between policies on Oracle scenario
"""

import csv
import matplotlib.pyplot as plt
import numpy as np

# Read summary CSV
data = []
with open('results/policy_comparison_summary.csv', 'r') as f:
    reader = csv.DictReader(f)
    for row in reader:
        data.append({
            'Policy': row['Policy'],
            'SuccessRate': float(row['SuccessRate']),
            'AvgLatency': float(row['AvgLatency']),
            'LocalCount': int(row['LocalCount']),
            'RemoteCount': int(row['RemoteCount']),
            'TotalEnergy': float(row['TotalEnergy'])
        })

policies = [d['Policy'] for d in data]
success_rates = [d['SuccessRate'] for d in data]
latencies = [d['AvgLatency'] * 1000 for d in data]  # Convert to ms
local_counts = [d['LocalCount'] for d in data]
remote_counts = [d['RemoteCount'] for d in data]
energies = [d['TotalEnergy'] for d in data]

# Colors
colors = {
    'Oracle': '#27AE60',      # Green (optimal)
    'Local': '#8E44AD',       # Purple
    'Random': '#3498DB',      # Blue
    'FirstRemote': '#E74C3C', # Red
    'Intelligent': '#F39C12'  # Orange
}
policy_colors = [colors.get(p, '#34495E') for p in policies]

# Create figure
fig, axes = plt.subplots(2, 2, figsize=(14, 10))
plt.subplots_adjust(hspace=0.3, wspace=0.3)

# Title
fig.suptitle('Policy Comparison on Oracle Scenario (Chaos Mode)', 
             fontsize=16, fontweight='bold', y=0.98)

# A. Success Rate
ax = axes[0, 0]
bars = ax.bar(policies, success_rates, color=policy_colors, edgecolor='white', linewidth=2)
ax.set_ylim(0, 110)
ax.set_ylabel('Success Rate (%)')
ax.set_title('A. Task Success Rate', fontweight='bold')
for bar, val in zip(bars, success_rates):
    ax.text(bar.get_x() + bar.get_width()/2, bar.get_height() + 2, 
           f'{val:.0f}%', ha='center', va='bottom', fontweight='bold')
ax.axhline(100, color='green', linestyle='--', alpha=0.5, label='Perfect')

# B. Average Latency
ax = axes[0, 1]
bars = ax.bar(policies, latencies, color=policy_colors, edgecolor='white', linewidth=2)
ax.set_ylabel('Average Latency (ms)')
ax.set_title('B. Processing Latency', fontweight='bold')
for bar, val in zip(bars, latencies):
    ax.text(bar.get_x() + bar.get_width()/2, bar.get_height() + 2, 
           f'{val:.1f}', ha='center', va='bottom', fontweight='bold', fontsize=9)
# Highlight best
min_idx = latencies.index(min(latencies))
bars[min_idx].set_edgecolor('gold')
bars[min_idx].set_linewidth(4)

# C. Offloading Strategy
ax = axes[1, 0]
x = np.arange(len(policies))
width = 0.35
bars1 = ax.bar(x - width/2, local_counts, width, label='Local', color='#95A5A6')
bars2 = ax.bar(x + width/2, remote_counts, width, label='Remote', color='#2C3E50')
ax.set_xticks(x)
ax.set_xticklabels(policies)
ax.set_ylabel('Number of Tasks')
ax.set_title('C. Offloading Strategy', fontweight='bold')
ax.legend()

# D. Energy Consumption
ax = axes[1, 1]
bars = ax.bar(policies, energies, color=policy_colors, edgecolor='white', linewidth=2)
ax.set_ylabel('Total Energy (J)')
ax.set_title('D. Energy Consumption', fontweight='bold')
for bar, val in zip(bars, energies):
    ax.text(bar.get_x() + bar.get_width()/2, bar.get_height() + 0.5, 
           f'{val:.1f}', ha='center', va='bottom', fontweight='bold', fontsize=9)
# Highlight best (lowest)
min_idx = energies.index(min(energies))
bars[min_idx].set_edgecolor('gold')
bars[min_idx].set_linewidth(4)

# Summary text
summary_text = f"""
Summary (23 tasks, Chaos Mode ON):
• All policies achieved 100% success on this scenario
• Oracle balanced: 12 local, 11 remote
• Lowest latency: FirstRemote ({min(latencies):.1f}ms)
• Lowest energy: Local/Intelligent ({min(energies):.1f}J)
"""
# fig.text(0.5, -0.02, summary_text, ha='center', fontsize=10, style='italic')

plt.savefig('policy_comparison.pdf', dpi=300, bbox_inches='tight', facecolor='white')
print("Saved: policy_comparison.pdf")

# Print summary
print("\n" + "="*60)
print("POLICY COMPARISON SUMMARY")
print("="*60)
print(f"{'Policy':<15} {'Success':>10} {'Latency(ms)':>12} {'Local':>8} {'Remote':>8} {'Energy(J)':>10}")
print("-"*60)
for d in data:
    print(f"{d['Policy']:<15} {d['SuccessRate']:>9.0f}% {d['AvgLatency']*1000:>12.1f} {d['LocalCount']:>8} {d['RemoteCount']:>8} {d['TotalEnergy']:>10.2f}")
print("="*60)
