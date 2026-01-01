#!/usr/bin/env python3
"""
Simple dashboard for single scenario results
Uses csv module directly to avoid pandas compatibility issues
"""

import sys
import csv
import matplotlib.pyplot as plt
import numpy as np

# Configuração visual
plt.style.use('seaborn-v0_8-whitegrid')
plt.rcParams['figure.facecolor'] = 'white'
plt.rcParams['axes.facecolor'] = '#f8f9fa'

def load_scenario_data(csv_path):
    """Load and parse scenario CSV data"""
    data = {'Time': [], 'EntityID': [], 'Metric': [], 'Value': [], 'Tag': [], 'TaskID': [], 'Location': []}
    
    with open(csv_path, 'r') as f:
        reader = csv.DictReader(f)
        for row in reader:
            data['Time'].append(float(row['Time']))
            data['EntityID'].append(int(row['EntityID']))
            data['Metric'].append(row['Metric'])
            try:
                data['Value'].append(float(row['Value']))
            except:
                data['Value'].append(0.0)
            data['Tag'].append(row['Tag'])
            try:
                data['TaskID'].append(int(row['TaskID']))
            except:
                data['TaskID'].append(-1)
            data['Location'].append(row['Location'])
    
    return data

def filter_by_metric(data, metric):
    """Filter data by metric name"""
    indices = [i for i, m in enumerate(data['Metric']) if m == metric]
    return {
        'Time': [data['Time'][i] for i in indices],
        'Value': [data['Value'][i] for i in indices],
        'TaskID': [data['TaskID'][i] for i in indices],
        'Tag': [data['Tag'][i] for i in indices],
    }

def plot_scenario_dashboard(data, output_path="scenario_dashboard.pdf"):
    """Create a dashboard for scenario results"""
    
    fig, axes = plt.subplots(2, 3, figsize=(18, 12))
    plt.subplots_adjust(hspace=0.35, wspace=0.3)
    
    # Colors
    success_color = '#27AE60'
    fail_color = '#E74C3C'
    local_color = '#3498DB'
    remote_color = '#9B59B6'
    
    # Get policy name
    task_success = filter_by_metric(data, 'TaskSuccess')
    policy_name = task_success['Tag'][0] if task_success['Tag'] else "Unknown"
    
    # === A. Task Success Timeline ===
    ax = axes[0, 0]
    if task_success['Value']:
        colors = [success_color if s == 1 else fail_color for s in task_success['Value']]
        ax.scatter(task_success['Time'], task_success['TaskID'], c=colors, s=80, alpha=0.8)
        ax.set_xlabel('Time (s)')
        ax.set_ylabel('Task ID')
        
        # Calculate success rate
        success_rate = sum(task_success['Value']) / len(task_success['Value']) * 100
        ax.set_title(f'A. Task Success Timeline\nSuccess Rate: {success_rate:.1f}%', fontweight='bold')
        
        # Legend
        ax.scatter([], [], c=success_color, s=80, label='Success')
        ax.scatter([], [], c=fail_color, s=80, label='Failed')
        ax.legend(loc='upper right')
    else:
        ax.text(0.5, 0.5, 'No TaskSuccess data', ha='center', va='center', transform=ax.transAxes)
    
    # === B. Task Latency Distribution ===
    ax = axes[0, 1]
    latency = filter_by_metric(data, 'TaskLatency')
    if latency['Value']:
        ax.hist(latency['Value'], bins=15, color='#3498DB', edgecolor='white', alpha=0.8)
        mean_lat = np.mean(latency['Value'])
        ax.axvline(mean_lat, color='#E74C3C', linestyle='--', linewidth=2, label=f'Mean: {mean_lat:.3f}s')
        ax.set_xlabel('Latency (s)')
        ax.set_ylabel('Count')
        ax.set_title('B. Task Latency Distribution', fontweight='bold')
        ax.legend()
    else:
        ax.text(0.5, 0.5, 'No TaskLatency data', ha='center', va='center', transform=ax.transAxes)
    
    # === C. Task Margin (Time to Deadline) ===
    ax = axes[0, 2]
    margin = filter_by_metric(data, 'TaskMargin')
    if margin['Value']:
        colors = [success_color if m > 0 else fail_color for m in margin['Value']]
        ax.bar(range(len(margin['Value'])), margin['Value'], color=colors, alpha=0.8)
        ax.axhline(0, color='black', linestyle='-', linewidth=1)
        ax.set_xlabel('Task Index')
        ax.set_ylabel('Time Margin (s)')
        ax.set_title('C. Time Margin to Deadline', fontweight='bold')
    else:
        ax.text(0.5, 0.5, 'No TaskMargin data', ha='center', va='center', transform=ax.transAxes)
    
    # === D. Queue Size Over Time ===
    ax = axes[1, 0]
    queue = filter_by_metric(data, 'QueueSize_Decision')
    if queue['Value']:
        ax.plot(queue['Time'], queue['Value'], marker='o', color='#8E44AD', linewidth=2, markersize=4)
        ax.fill_between(queue['Time'], queue['Value'], alpha=0.3, color='#8E44AD')
        ax.set_xlabel('Time (s)')
        ax.set_ylabel('Queue Size')
        ax.set_title('D. Decision Queue Size', fontweight='bold')
    else:
        ax.text(0.5, 0.5, 'No QueueSize data', ha='center', va='center', transform=ax.transAxes)
    
    # === E. Offloading Decisions ===
    ax = axes[1, 1]
    offload = filter_by_metric(data, 'OffloadingType')
    if offload['Value']:
        local_count = sum(1 for v in offload['Value'] if v == 0)
        remote_count = sum(1 for v in offload['Value'] if v == 1)
        
        bars = ax.bar(['Local', 'Remote'], [local_count, remote_count], 
                      color=[local_color, remote_color], edgecolor='white', linewidth=2)
        ax.set_ylabel('Number of Tasks')
        ax.set_title('E. Offloading Strategy', fontweight='bold')
        
        # Add values on bars
        for bar, val in zip(bars, [local_count, remote_count]):
            ax.text(bar.get_x() + bar.get_width()/2, bar.get_height() + 0.5, 
                   str(val), ha='center', va='bottom', fontweight='bold', fontsize=14)
    else:
        ax.text(0.5, 0.5, 'No OffloadingType data', ha='center', va='center', transform=ax.transAxes)
    
    # === F. Battery Level ===
    ax = axes[1, 2]
    battery = filter_by_metric(data, 'BatteryRemaining')
    if battery['Value']:
        ax.plot(battery['Time'], battery['Value'], marker='o', 
                color='#F39C12', linewidth=2, markersize=4)
        ax.fill_between(battery['Time'], battery['Value'], alpha=0.3, color='#F39C12')
        ax.set_xlabel('Time (s)')
        ax.set_ylabel('Battery (J)')
        
        # Energy consumed
        energy_consumed = battery['Value'][0] - battery['Value'][-1]
        ax.set_title(f'F. Battery Level\nTotal Energy: {energy_consumed:.2f}J', fontweight='bold')
    else:
        ax.text(0.5, 0.5, 'No Battery data', ha='center', va='center', transform=ax.transAxes)
    
    # Main title
    plt.suptitle(f'Oracle Scenario Dashboard (Chaos Mode)\nPolicy: {policy_name}', 
                 fontsize=18, fontweight='bold', y=0.98)
    
    # Save
    plt.savefig(output_path, dpi=300, bbox_inches='tight', facecolor='white')
    print(f"Dashboard saved: {output_path}")
    
    # Print summary
    print("\n" + "="*50)
    print("SCENARIO SUMMARY")
    print("="*50)
    
    if task_success['Value']:
        total_tasks = len(task_success['Value'])
        successful = sum(task_success['Value'])
        print(f"Total Tasks: {total_tasks}")
        print(f"Successful: {int(successful)} ({successful/total_tasks*100:.1f}%)")
        print(f"Failed: {int(total_tasks - successful)} ({(1-successful/total_tasks)*100:.1f}%)")
    
    if latency['Value']:
        print(f"\nLatency (s):")
        print(f"  Mean: {np.mean(latency['Value']):.4f}")
        print(f"  Std:  {np.std(latency['Value']):.4f}")
        print(f"  Max:  {max(latency['Value']):.4f}")
    
    if offload['Value']:
        print(f"\nOffloading:")
        print(f"  Local:  {local_count}")
        print(f"  Remote: {remote_count}")
    
    print("="*50)

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python3 scripts/plot_scenario.py <csv_file> [output.pdf]")
        print("Example: python3 scripts/plot_scenario.py results/scenario_OracleOptimal_1978.csv")
        sys.exit(1)
    
    csv_file = sys.argv[1]
    output_file = sys.argv[2] if len(sys.argv) > 2 else "scenario_dashboard.pdf"
    
    data = load_scenario_data(csv_file)
    plot_scenario_dashboard(data, output_file)
