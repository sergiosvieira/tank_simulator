import matplotlib.pyplot as plt
import numpy as np

# Data gathered from runs
policies = ['Oracle', 'Local', 'Random', 'FirstRemote', 'Intelligent']

# Success Rates (%)
no_chaos_success = [100, 10, 52, 62.5, 71]
chaos_success =    [100, 10, 46, 56.0, 65]

x = np.arange(len(policies))
width = 0.35

fig, ax = plt.subplots(figsize=(10, 6))
rects1 = ax.bar(x - width/2, no_chaos_success, width, label='No Chaos (Stable)', color='#3498db')
rects2 = ax.bar(x + width/2, chaos_success, width, label='Chaos Mode (High Jitter)', color='#e74c3c')

ax.set_ylabel('Success Rate (%)')
ax.set_title('Impact of Chaos (Network Instability) on Policy Performance')
ax.set_xticks(x)
ax.set_xticklabels(policies)
ax.legend()
ax.set_ylim(0, 110)
ax.grid(axis='y', linestyle='--', alpha=0.7)

# Add labels
def autolabel(rects):
    for rect in rects:
        height = rect.get_height()
        ax.annotate(f'{height}%',
                    xy=(rect.get_x() + rect.get_width() / 2, height),
                    xytext=(0, 3), 
                    textcoords="offset points",
                    ha='center', va='bottom', fontsize=10, fontweight='bold')

autolabel(rects1)
autolabel(rects2)

plt.tight_layout()
plt.savefig('chaos_impact_comparison.pdf')
print("Comparison plot saved to chaos_impact_comparison.pdf")
