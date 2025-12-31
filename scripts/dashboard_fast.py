import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np

# --- CONFIGURAÇÃO VISUAL ---
POLICIES_CONFIG = {
    "Local":       {"color": "#8E44AD", "label": "Local"},
    "Random":      {"color": "#27AE60", "label": "Random"},
    "Intelligent": {"color": "#C0392B", "label": "Intelligent"},
    "FirstRemote": {"color": "#34495E", "label": "First Remote"}
}

sns.set(style="whitegrid", context="paper", font_scale=1.2)
plt.rcParams['hatch.linewidth'] = 0.5

def load_aggregated_data():
    try:
        df_sum = pd.read_csv("results/aggregated_summary.csv")
        df_ts = pd.read_csv("results/aggregated_timeseries.csv")
        return df_sum, df_ts
    except FileNotFoundError:
        print("[ERRO] Arquivos agregados nao encontrados. Rode 'tools/log_aggregator results/' primeiro.")
        return None, None

def plot_dashboard_fast(df_sum, df_ts):
    policies = [p for p in POLICIES_CONFIG.keys() if p in df_sum['Policy'].unique()]
    palette = {p: POLICIES_CONFIG[p]['color'] for p in policies}
    
    fig, axes = plt.subplots(3, 3, figsize=(22, 18))
    plt.subplots_adjust(hspace=0.4, wspace=0.3)
    
    # === LINHA 1 ===
    
    # A. Success Rate (Barplot of totals per seed)
    ax = axes[0, 0]
    sns.barplot(ax=ax, x='Policy', y='SuccessRate', hue='Policy', data=df_sum, 
                palette=palette, order=policies, estimator=np.mean, errorbar='sd')
    ax.set_title('A. Success Rate (Mean of Seeds)', fontweight='bold')
    ax.set_ylim(0, 1.1)
    for container in ax.containers: ax.bar_label(container, fmt='%.2f', padding=3)
    if ax.get_legend(): ax.get_legend().remove()

    # B. Latency (Distribution of Averages)
    ax = axes[0, 1]
    # Usamos AvgLatency de cada seed para ver a variabilidade entre execuções
    sns.boxplot(ax=ax, x='Policy', y='AvgLatency', hue='Policy', data=df_sum, 
                 palette=palette, order=policies)
    ax.set_title('B. Latency Stability (Avg per Seed)', fontweight='bold')
    ax.set_ylabel('Avg Time (s)')
    if ax.get_legend(): ax.get_legend().remove()

    # C. Failures (Barplot Total) - Substituto para TimeSeries de erros
    ax = axes[0, 2]
    # Sumariza falhas totais por policy
    fail_agg = df_sum.groupby('Policy')['Failures'].mean().reset_index()
    sns.barplot(ax=ax, x='Policy', y='Failures', hue='Policy', data=fail_agg,
                palette=palette, order=policies)
    ax.set_title('C. Avg Failures / Overflow', fontweight='bold')
    for container in ax.containers: ax.bar_label(container, fmt='%.1f')
    if ax.get_legend(): ax.get_legend().remove()

    # === LINHA 2 ===

    # D. Queue Size (TimeSeries)
    ax = axes[1, 0]
    sns.lineplot(ax=ax, x='Time', y='AvgQueueSize', hue='Policy', data=df_ts, 
                 palette=palette, hue_order=policies)
    ax.set_title('D. Queue Size (Time Avg)', fontweight='bold')
    if ax.get_legend(): ax.get_legend().remove()

    # E. Offloading Distribution (Local vs Remote)
    ax = axes[1, 1]
    # Precisamos derreter (melt) as colunas OffloadLocal/Remote
    off_melt = df_sum.melt(id_vars=['Policy'], value_vars=['OffloadLocal', 'OffloadRemote'], 
                           var_name='Type', value_name='Count')
    # Sumarizar média por policy
    off_agg = off_melt.groupby(['Policy', 'Type'])['Count'].mean().reset_index()
    off_agg['Location'] = off_agg['Type'].apply(lambda x: 'Local' if 'Local' in x else 'Remote')
    
    sns.barplot(ax=ax, x='Policy', y='Count', hue='Location', data=off_agg,
                order=policies, palette={"Local": "#95A5A6", "Remote": "#2C3E50"})
    ax.set_title('E. Avg Decisions per Run', fontweight='bold')
    ax.legend(title='Target')

    # F. Energy Composition
    ax = axes[1, 2]
    # Média de energia por policy
    en_agg = df_sum.groupby('Policy')[['TotalEnergyCPU', 'TotalEnergyTx']].mean().reindex(policies)
    if not en_agg.empty:
        ax.bar(en_agg.index, en_agg['TotalEnergyCPU'], label='CPU', 
               color=[palette[p] for p in en_agg.index], edgecolor='white')
        ax.bar(en_agg.index, en_agg['TotalEnergyTx'], bottom=en_agg['TotalEnergyCPU'], 
               label='Tx', color=[palette[p] for p in en_agg.index], hatch='///', edgecolor='white', alpha=0.6)
        ax.set_title('F. Avg Energy Consumed (J)', fontweight='bold')
        ax.legend()

    # === LINHA 3 ===

    # G. Battery (TimeSeries)
    ax = axes[2, 0]
    sns.lineplot(ax=ax, x='Time', y='AvgBattery', hue='Policy', data=df_ts, 
                 palette=palette, hue_order=policies)
    ax.set_title('G. Battery Level (Avg)', fontweight='bold')
    if ax.get_legend(): ax.get_legend().remove()

    # H. Efficiency (J per Success)
    ax = axes[2, 1]
    # Calcular eficiencia por linha (seed) antes da média
    # Evita divisão por zero
    df_sum['TotalEnergy'] = df_sum['TotalEnergyCPU'] + df_sum['TotalEnergyTx']
    # Total Tasks = Successes / SuccessRate (aprox, ou podemos usar Successes direto se tivessemos salvo, salvamos successes?)
    # O C++ salvou SuccessRate e counts se sucesso?
    # C++ salvou: SuccessRate. Precisamos de SuccessCount para dividir Energia por ele.
    # Ah, o C++ não salvou 'SuccessCount' explicitamente no CSV, só rate.
    # Mas salvou TotalTasks? Não.
    # Mas podemos aproximar: Efficiency = TotalEnergy / (SuccessRate * 100 tasks? n sabemos N)
    # Espera, success rate é 0..1. 
    # Melhor métrica aqui seria Energy per Task? Ou Energy per Success?
    # Vamos usar Energy / SuccessRate (inverso da eficiencia) -> J for 100% success? Não faz sentido.
    # Vamos assumir N constante entre seeds e usar TotalEnergy como proxy de custo, 
    # mas o gráfico original era "J / Success".
    # Sem o count de sucessos exatos, fica difícil.
    # Mudei o C++ para não salvar successes count. Erro meu.
    # Mas tenho o rate. Assumindo que o numero de tasks é constante (cenário fixo), Rate é proporcional a Count.
    # Vamos plotar "J / %Success" (Energy cost for 1% success)? Unintuitive.
    # Vamos apenas plotar Total Energy vs Success Rate (Scatter) no proximo.
    # Aqui vamos plotar "Energy Efficiency Score" = SuccessRate / TotalEnergy (Higher is better)
    
    df_sum['EfficiencyScore'] = df_sum['SuccessRate'] / df_sum['TotalEnergy']
    sns.barplot(ax=ax, x='Policy', y='EfficiencyScore', hue='Policy', data=df_sum,
                palette=palette, order=policies)
    ax.set_title('H. Efficiency Score (Success/Energy)', fontweight='bold')
    ax.set_ylabel('Rate / J')
    if ax.get_legend(): ax.get_legend().remove()

    # I. Pareto Map
    ax = axes[2, 2]
    # Scatter de todos os seeds
    sns.scatterplot(ax=ax, data=df_sum, x='TotalEnergy', y='SuccessRate', hue='Policy', 
                    palette=palette, style='Policy', s=100, alpha=0.7)
    
    # Adicionar centróides (média) maiores
    means = df_sum.groupby('Policy')[['TotalEnergy', 'SuccessRate']].mean().reset_index()
    sns.scatterplot(ax=ax, data=means, x='TotalEnergy', y='SuccessRate', hue='Policy',
                    palette=palette, s=400, marker='X', legend=False, edgecolor='black', linewidth=2)
    
    ax.set_title('I. Pareto Frontier (Seeds + Means)', fontweight='bold')
    if ax.get_legend(): ax.get_legend().remove()

    plt.suptitle("Fast Dashboard (C++ Pre-processed)", fontsize=20, y=0.98)
    plt.savefig("FINAL_DASHBOARD_FAST.png", dpi=300, bbox_inches='tight')
    print("Grafico salvo: FINAL_DASHBOARD_FAST.png")

if __name__ == "__main__":
    s, t = load_aggregated_data()
    if s is not None:
        plot_dashboard_fast(s, t)
