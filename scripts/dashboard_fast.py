#!/usr/bin/env python3
"""
IEEE-Style Individual Figures for Offloading Policy Comparison
Generates separate publication-ready PDF files with descriptive titles and confidence intervals.
"""

import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import os

# =============================================================================
# IEEE STYLE CONFIGURATION
# =============================================================================

plt.rcParams['font.family'] = 'serif'
plt.rcParams['font.serif'] = ['Times New Roman', 'DejaVu Serif', 'Liberation Serif']
plt.rcParams['font.size'] = 11
plt.rcParams['axes.titlesize'] = 12
plt.rcParams['axes.labelsize'] = 11
plt.rcParams['xtick.labelsize'] = 10
plt.rcParams['ytick.labelsize'] = 10
plt.rcParams['legend.fontsize'] = 10

# Professional color palette
COLORS = {
    "Oracle":      "#1f77b4",  # Blue
    "Local":       "#d62728",  # Red
    "Random":      "#2ca02c",  # Green
    "Intelligent": "#ff7f0e",  # Orange
    "FirstRemote": "#9467bd",  # Purple
}

# Larger figure size for better readability
FIG_WIDTH = 6
FIG_HEIGHT = 4.5

OUTPUT_DIR = "figures"

def load_aggregated_data():
    """Load pre-aggregated CSV files."""
    try:
        df_sum = pd.read_csv("results/aggregated_summary.csv")
        df_ts = pd.read_csv("results/aggregated_timeseries.csv")
        return df_sum, df_ts
    except FileNotFoundError:
        print("[ERROR] Aggregated files not found.")
        return None, None

def add_value_labels(ax, rects, fmt='%.1f', padding=5, errors=None):
    """Add value labels above bars (and above error bars if provided)."""
    for i, rect in enumerate(rects):
        height = rect.get_height()
        if height > 0:
            # If errors provided, position label above the error bar
            label_y = height
            if errors is not None and i < len(errors):
                label_y = height + errors[i]
            
            ax.annotate(fmt % height,
                        xy=(rect.get_x() + rect.get_width() / 2, label_y),
                        xytext=(0, padding),
                        textcoords="offset points",
                        ha='center', va='bottom', fontsize=9, fontweight='bold')

def get_mean_std(df, policy_col, value_col, policies):
    """Calculate mean and std for each policy."""
    means = []
    stds = []
    for p in policies:
        data = df[df[policy_col] == p][value_col]
        means.append(data.mean())
        stds.append(data.std())
    return means, stds

def plot_success_rate(df_sum, policies, palette):
    """Task Completion Rate with Confidence Intervals"""
    fig, ax = plt.subplots(figsize=(FIG_WIDTH, FIG_HEIGHT))
    
    means, stds = get_mean_std(df_sum, 'Policy', 'SuccessRate', policies)
    means = [m * 100 for m in means]  # Convert to percentage
    stds = [s * 100 for s in stds]
    
    x = np.arange(len(policies))
    bars = ax.bar(x, means, yerr=stds, capsize=5, 
                  color=[palette[p] for p in policies],
                  edgecolor='black', linewidth=0.8, error_kw={'linewidth': 1.5})
    
    ax.set_ylabel('Success Rate (%)')
    ax.set_xlabel('Offloading Policy')
    ax.set_title('Percentage of Tasks Completed Within Deadline\n(Success = task finished before deadline expired)', fontweight='bold')
    ax.set_ylim(0, 120)
    ax.set_xticks(x)
    ax.set_xticklabels(policies)
    ax.yaxis.grid(True, linestyle='--', alpha=0.7)
    add_value_labels(ax, bars, errors=stds)
    
    plt.tight_layout()
    fig.savefig(os.path.join(OUTPUT_DIR, "fig1_success_rate.pdf"), dpi=300, bbox_inches='tight')
    print("Saved: fig1_success_rate.pdf")
    plt.close(fig)

def plot_latency(df_sum, policies, palette):
    """Average Task Latency with Confidence Intervals"""
    fig, ax = plt.subplots(figsize=(FIG_WIDTH, FIG_HEIGHT))
    
    means, stds = get_mean_std(df_sum, 'Policy', 'AvgLatency', policies)
    # Values are already in seconds
    
    x = np.arange(len(policies))
    bars = ax.bar(x, means, yerr=stds, capsize=5,
                  color=[palette[p] for p in policies],
                  edgecolor='black', linewidth=0.8, error_kw={'linewidth': 1.5})
    
    ax.set_ylabel('Latency (s)')
    ax.set_xlabel('Offloading Policy')
    ax.set_title('Average Task Latency (Creation to Completion)\n(Includes queue wait + processing + transfer time)', fontweight='bold')
    ax.set_xticks(x)
    ax.set_xticklabels(policies)
    ax.yaxis.grid(True, linestyle='--', alpha=0.7)
    add_value_labels(ax, bars, errors=stds)
    
    plt.tight_layout()
    fig.savefig(os.path.join(OUTPUT_DIR, "fig2_latency.pdf"), dpi=300, bbox_inches='tight')
    print("Saved: fig2_latency.pdf")
    plt.close(fig)

def plot_transfer_time(df_sum, policies, palette):
    """Network Transfer Overhead"""
    fig, ax = plt.subplots(figsize=(FIG_WIDTH, FIG_HEIGHT))
    
    if 'AvgTransferTime' in df_sum.columns:
        means, stds = get_mean_std(df_sum, 'Policy', 'AvgTransferTime', policies)
        # Values are in seconds
        
        x = np.arange(len(policies))
        bars = ax.bar(x, means, yerr=stds, capsize=5,
                      color=[palette[p] for p in policies],
                      edgecolor='black', linewidth=0.8, error_kw={'linewidth': 1.5})
        add_value_labels(ax, bars, fmt='%.3f', errors=stds)
        ax.set_xticks(x)
        ax.set_xticklabels(policies)
    else:
        ax.text(0.5, 0.5, 'Transfer Time Data Not Available', ha='center', va='center', fontsize=12)
        ax.set_xticks([])
    
    ax.set_ylabel('Transfer Time (s)')
    ax.set_xlabel('Offloading Policy')
    ax.set_title('Average Data Transfer Time to RSU\n(Time to upload task input data over network)', fontweight='bold')
    ax.yaxis.grid(True, linestyle='--', alpha=0.7)
    
    plt.tight_layout()
    fig.savefig(os.path.join(OUTPUT_DIR, "fig3_transfer_time.pdf"), dpi=300, bbox_inches='tight')
    print("Saved: fig3_transfer_time.pdf")
    plt.close(fig)

def plot_queue_size(df_ts, policies, palette):
    """Decision Queue Evolution"""
    fig, ax = plt.subplots(figsize=(FIG_WIDTH, FIG_HEIGHT))
    
    for policy in policies:
        data = df_ts[df_ts['Policy'] == policy]
        ax.plot(data['Time'], data['AvgQueueSize'], label=policy, 
                color=palette[policy], linewidth=2)
    
    ax.set_xlabel('Simulation Time (s)')
    ax.set_ylabel('Average Queue Size (tasks)')
    ax.set_title('Decision Queue Length Over Time\n(Tasks waiting for offloading policy decision)', fontweight='bold')
    ax.legend(loc='best', framealpha=0.9)
    ax.yaxis.grid(True, linestyle='--', alpha=0.7)
    ax.xaxis.grid(True, linestyle='--', alpha=0.7)
    
    plt.tight_layout()
    fig.savefig(os.path.join(OUTPUT_DIR, "fig4_decision_queue.pdf"), dpi=300, bbox_inches='tight')
    print("Saved: fig4_decision_queue.pdf")
    plt.close(fig)

def plot_processing_queue(df_ts, policies, palette):
    """Processing Queue Evolution"""
    fig, ax = plt.subplots(figsize=(FIG_WIDTH, FIG_HEIGHT))
    
    if 'AvgQueueProc' not in df_ts.columns:
        ax.text(0.5, 0.5, 'Processing Queue Data Not Available\n(Re-run simulation and log_aggregator)', 
                ha='center', va='center', fontsize=10, transform=ax.transAxes)
    else:
        for policy in policies:
            data = df_ts[df_ts['Policy'] == policy]
            ax.plot(data['Time'], data['AvgQueueProc'], label=policy, 
                    color=palette[policy], linewidth=2)
        ax.legend(loc='best', framealpha=0.9)
    
    ax.set_xlabel('Simulation Time (s)')
    ax.set_ylabel('Average Queue Size (tasks)')
    ax.set_title('CPU Processing Queue Length Over Time\n(Tasks waiting for local CPU execution, max=10)', fontweight='bold')
    ax.yaxis.grid(True, linestyle='--', alpha=0.7)
    ax.xaxis.grid(True, linestyle='--', alpha=0.7)
    
    plt.tight_layout()
    fig.savefig(os.path.join(OUTPUT_DIR, "fig10_processing_queue.pdf"), dpi=300, bbox_inches='tight')
    print("Saved: fig10_processing_queue.pdf")
    plt.close(fig)

def plot_offloading_strategy(df_sum, policies, palette):
    """Offloading Decision Distribution"""
    fig, ax = plt.subplots(figsize=(FIG_WIDTH, FIG_HEIGHT))
    
    x = np.arange(len(policies))
    width = 0.35
    
    local_means, local_stds = get_mean_std(df_sum, 'Policy', 'OffloadLocal', policies)
    remote_means, remote_stds = get_mean_std(df_sum, 'Policy', 'OffloadRemote', policies)
    
    bars1 = ax.bar(x - width/2, local_means, width, yerr=local_stds, capsize=4,
                   label='Local Execution', color='#7f7f7f', edgecolor='black', linewidth=0.8)
    bars2 = ax.bar(x + width/2, remote_means, width, yerr=remote_stds, capsize=4,
                   label='Remote Offloading', color='#2c3e50', edgecolor='black', linewidth=0.8)
    
    ax.set_ylabel('Number of Tasks')
    ax.set_xlabel('Offloading Policy')
    ax.set_title('Offloading Decision Count per Policy\n(How many tasks were executed locally vs. sent to RSU)', fontweight='bold')
    ax.set_xticks(x)
    ax.set_xticklabels(policies)
    ax.legend(loc='upper right', framealpha=0.9)
    ax.yaxis.grid(True, linestyle='--', alpha=0.7)
    
    # Value labels
    for bars in [bars1, bars2]:
        for bar in bars:
            h = bar.get_height()
            if h > 0:
                ax.annotate(f'{int(h)}', xy=(bar.get_x() + bar.get_width()/2, h),
                            xytext=(0, 5), textcoords="offset points",
                            ha='center', fontsize=9, fontweight='bold')
    
    plt.tight_layout()
    fig.savefig(os.path.join(OUTPUT_DIR, "fig5_offloading_strategy.pdf"), dpi=300, bbox_inches='tight')
    print("Saved: fig5_offloading_strategy.pdf")
    plt.close(fig)

def plot_energy_breakdown(df_sum, policies, palette):
    """Energy Consumption Breakdown"""
    fig, ax = plt.subplots(figsize=(FIG_WIDTH, FIG_HEIGHT))
    
    cpu_means, _ = get_mean_std(df_sum, 'Policy', 'TotalEnergyCPU', policies)
    tx_means, _ = get_mean_std(df_sum, 'Policy', 'TotalEnergyTx', policies)
    
    x = np.arange(len(policies))
    bars1 = ax.bar(x, cpu_means, label='CPU Processing Energy',
                   color=[palette[p] for p in policies], edgecolor='black', linewidth=0.8)
    bars2 = ax.bar(x, tx_means, bottom=cpu_means, label='Transmission Energy',
                   color=[palette[p] for p in policies], edgecolor='black', linewidth=0.8,
                   hatch='///', alpha=0.7)
    
    ax.set_ylabel('Total Energy (Joules)')
    ax.set_xlabel('Offloading Policy')
    ax.set_title('Cumulative Energy Consumption per Policy\n(Stacked: CPU processing energy + wireless transmission energy)', fontweight='bold')
    ax.set_xticks(x)
    ax.set_xticklabels(policies)
    ax.legend(loc='upper right', framealpha=0.9)
    ax.yaxis.grid(True, linestyle='--', alpha=0.7)
    
    # Total labels on top
    for i, (c, t) in enumerate(zip(cpu_means, tx_means)):
        total = c + t
        ax.annotate(f'{total:.1f}J', xy=(i, total), xytext=(0, 5),
                    textcoords="offset points", ha='center', fontsize=9, fontweight='bold')
    
    plt.tight_layout()
    fig.savefig(os.path.join(OUTPUT_DIR, "fig6_energy_breakdown.pdf"), dpi=300, bbox_inches='tight')
    print("Saved: fig6_energy_breakdown.pdf")
    plt.close(fig)

def plot_battery(df_ts, policies, palette):
    """Battery Depletion Over Time"""
    fig, ax = plt.subplots(figsize=(FIG_WIDTH, FIG_HEIGHT))
    
    for policy in policies:
        data = df_ts[df_ts['Policy'] == policy]
        ax.plot(data['Time'], data['AvgBattery'], label=policy,
                color=palette[policy], linewidth=2)
    
    ax.set_xlabel('Simulation Time (s)')
    ax.set_ylabel('Remaining Battery (Joules)')
    ax.set_title('Vehicle Battery Depletion Over Time\n(Initial battery capacity minus consumed energy)', fontweight='bold')
    ax.legend(loc='best', framealpha=0.9)
    ax.yaxis.grid(True, linestyle='--', alpha=0.7)
    ax.xaxis.grid(True, linestyle='--', alpha=0.7)
    
    plt.tight_layout()
    fig.savefig(os.path.join(OUTPUT_DIR, "fig7_battery.pdf"), dpi=300, bbox_inches='tight')
    print("Saved: fig7_battery.pdf")
    plt.close(fig)

def plot_efficiency(df_sum, policies, palette):
    """System Efficiency Score"""
    fig, ax = plt.subplots(figsize=(FIG_WIDTH, FIG_HEIGHT))
    
    df_sum['TotalEnergy'] = df_sum['TotalEnergyCPU'] + df_sum['TotalEnergyTx']
    df_sum['Efficiency'] = df_sum['SuccessRate'] / df_sum['TotalEnergy']
    
    means, stds = get_mean_std(df_sum, 'Policy', 'Efficiency', policies)
    
    x = np.arange(len(policies))
    bars = ax.bar(x, means, yerr=stds, capsize=5,
                  color=[palette[p] for p in policies],
                  edgecolor='black', linewidth=0.8, error_kw={'linewidth': 1.5})
    
    ax.set_ylabel('Efficiency (Success Rate / Joule)')
    ax.set_xlabel('Offloading Policy')
    ax.set_title('Energy Efficiency: Success Rate per Joule Consumed\n(Higher = better trade-off between success and energy)', fontweight='bold')
    ax.set_xticks(x)
    ax.set_xticklabels(policies)
    ax.yaxis.grid(True, linestyle='--', alpha=0.7)
    add_value_labels(ax, bars, fmt='%.4f', errors=stds)
    
    plt.tight_layout()
    fig.savefig(os.path.join(OUTPUT_DIR, "fig8_efficiency.pdf"), dpi=300, bbox_inches='tight')
    print("Saved: fig8_efficiency.pdf")
    plt.close(fig)

def plot_pareto(df_sum, policies, palette):
    """Pareto Frontier: Energy vs Success Trade-off"""
    fig, ax = plt.subplots(figsize=(FIG_WIDTH, FIG_HEIGHT))
    
    df_sum['TotalEnergy'] = df_sum['TotalEnergyCPU'] + df_sum['TotalEnergyTx']
    
    for policy in policies:
        data = df_sum[df_sum['Policy'] == policy]
        # Individual points (seeds)
        ax.scatter(data['TotalEnergy'], data['SuccessRate'] * 100,
                   label=policy, color=palette[policy], s=60, alpha=0.6,
                   edgecolors='black', linewidth=0.5)
        
        # Mean marker (larger X)
        mean_e = data['TotalEnergy'].mean()
        mean_s = data['SuccessRate'].mean() * 100
        std_e = data['TotalEnergy'].std()
        std_s = data['SuccessRate'].std() * 100
        
        ax.errorbar(mean_e, mean_s, xerr=std_e, yerr=std_s, 
                    color=palette[policy], fmt='X', markersize=12,
                    capsize=4, capthick=1.5, elinewidth=1.5, markeredgecolor='black')
    
    ax.set_xlabel('Total Energy Consumed (Joules)')
    ax.set_ylabel('Task Success Rate (%)')
    ax.set_title('Energy vs. Success Rate Trade-off Analysis\n(Ideal: upper-left corner = high success, low energy)', fontweight='bold')
    ax.legend(loc='lower right', framealpha=0.9)
    ax.grid(True, linestyle='--', alpha=0.5)
    
    plt.tight_layout()
    fig.savefig(os.path.join(OUTPUT_DIR, "fig9_pareto.pdf"), dpi=300, bbox_inches='tight')
    print("Saved: fig9_pareto.pdf")
    plt.close(fig)

def main():
    os.makedirs(OUTPUT_DIR, exist_ok=True)
    
    df_sum, df_ts = load_aggregated_data()
    if df_sum is None:
        return
    
    policies = [p for p in COLORS.keys() if p in df_sum['Policy'].unique()]
    palette = {p: COLORS[p] for p in policies}
    
    print(f"Generating figures for policies: {policies}")
    print(f"Output directory: {OUTPUT_DIR}/")
    print("-" * 50)
    
    plot_success_rate(df_sum, policies, palette)
    plot_latency(df_sum, policies, palette)
    plot_transfer_time(df_sum, policies, palette)
    plot_queue_size(df_ts, policies, palette)
    plot_processing_queue(df_ts, policies, palette)
    plot_offloading_strategy(df_sum, policies, palette)
    plot_energy_breakdown(df_sum, policies, palette)
    plot_battery(df_ts, policies, palette)
    plot_efficiency(df_sum, policies, palette)
    plot_pareto(df_sum, policies, palette)
    
    print("-" * 50)
    print(f"All 10 figures saved to {OUTPUT_DIR}/")

if __name__ == "__main__":
    main()
