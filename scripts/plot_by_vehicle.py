import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import argparse
import os

def setup_plot_style():
    """Configura o estilo dos gráficos para padrões de publicação (IEEE/ACM)."""
    sns.set_theme(style="whitegrid", context="paper")
    plt.rcParams.update({
        'font.family': 'sans-serif',
        'font.size': 12,
        'axes.titlesize': 14,
        'axes.labelsize': 12,
        'xtick.labelsize': 10,
        'ytick.labelsize': 10,
        'legend.fontsize': 10,
        'figure.figsize': (10, 6),
        'figure.dpi': 120,
        'lines.linewidth': 2
    })

def load_data(filepath):
    if not os.path.exists(filepath):
        print(f"Erro: Arquivo não encontrado em {filepath}")
        return None
    return pd.read_csv(filepath)

def plot_latency_cdf_per_vehicle(df, output_dir):
    """
    Gera a CDF da Latência por veículo.
    Crucial para analisar violação de deadlines (Tail Latency).
    Equivalent to 'plot_latency_cdf' in original script but per vehicle.
    """
    data = df[df['Metric'] == 'TaskLatency'].copy()
    if data.empty: return
    
    vehicles = data['EntityID'].unique()
    for v_id in vehicles:
        v_data = data[data['EntityID'] == v_id]
        if v_data.empty: continue

        plt.figure()
        if v_data['Tag'].nunique() > 1:
            sns.ecdfplot(data=v_data, x='Value', hue='Tag', linewidth=2)
        else:
            sns.ecdfplot(data=v_data, x='Value', linewidth=2, color='blue')

        plt.axvline(0.1, color='red', linestyle='--', label='Deadline (100ms)')
        plt.title(f'CDF de Latência - Veículo {v_id}')
        plt.xlabel('Latência (s)')
        plt.ylabel('Probabilidade P(X <= x)')
        plt.legend(loc='lower right')
        plt.grid(True, which='both', linestyle='--', alpha=0.5)
        plt.tight_layout()
        plt.savefig(os.path.join(output_dir, f'cdf_latency_vehicle_{v_id}.png'))
        plt.close()

def plot_queue_dynamics_per_vehicle(df, output_dir):
    """
    Analisa o tamanho da fila ao longo do tempo para cada veículo.
    Equivalent to 'plot_queue_dynamics'.
    """
    data = df[df['Metric'] == 'QueueSize_Decision'].copy()
    if data.empty: return

    vehicles = data['EntityID'].unique()
    for v_id in vehicles:
        v_data = data[data['EntityID'] == v_id]
        if v_data.empty: continue

        plt.figure()
        # Raw data
        sns.lineplot(data=v_data, x='Time', y='Value', alpha=0.3, label='Instantâneo')
        
        # Trend
        trend = v_data.groupby('Time')['Value'].mean().rolling(window=20).mean()
        plt.plot(trend.index, trend.values, 'k-', linewidth=2, label='Média Móvel')

        plt.title(f'Dinâmica da Fila - Veículo {v_id}')
        plt.xlabel('Tempo de Simulação (s)')
        plt.ylabel('Tamanho da Fila (Pacotes)')
        plt.legend()
        plt.tight_layout()
        plt.savefig(os.path.join(output_dir, f'queue_dynamics_vehicle_{v_id}.png'))
        plt.close()

def plot_success_rate_per_vehicle(df, output_dir):
    """
    Mostra a taxa de sucesso no tempo por veículo.
    Equivalent to 'plot_success_rate'.
    """
    data = df[df['Metric'] == 'TaskSuccess'].copy()
    if data.empty: return

    vehicles = data['EntityID'].unique()
    for v_id in vehicles:
        v_data = data[data['EntityID'] == v_id].copy()
        if v_data.empty: continue
        
        # Time bins like original
        v_data['TimeBin'] = (v_data['Time'] // 0.5) * 0.5
        grouped = v_data.groupby('TimeBin')['Value'].mean().reset_index()

        plt.figure()
        sns.lineplot(data=grouped, x='TimeBin', y='Value', marker='o')
        plt.axhline(0.95, color='green', linestyle=':', label='Meta SLA (95%)')
        plt.title(f'Taxa de Sucesso no Tempo - Veículo {v_id}')
        plt.xlabel('Tempo (s)')
        plt.ylabel('Taxa de Sucesso (0.0 - 1.0)')
        plt.ylim(-0.05, 1.05)
        plt.legend(loc='lower right')
        plt.tight_layout()
        plt.savefig(os.path.join(output_dir, f'success_rate_vehicle_{v_id}.png'))
        plt.close()

def plot_offloading_stats_per_vehicle(df, output_dir):
    """
    Mostra estatísticas de offloading por veículo.
    Equivalent to 'plot_offloading_stats'.
    """
    data = df[df['Metric'] == 'OffloadingType'].copy()
    if data.empty: return

    vehicles = data['EntityID'].unique()
    for v_id in vehicles:
        v_data = data[data['EntityID'] == v_id]
        if v_data.empty: continue

        plt.figure(figsize=(8, 5))
        ax = sns.countplot(data=v_data, x='Tag')
        plt.title(f'Distribuição de Decisões - Veículo {v_id}')
        plt.xlabel('Destino')
        plt.ylabel('Quantidade de Tarefas')
        
        for container in ax.containers:
            ax.bar_label(container)
            
        plt.tight_layout()
        plt.savefig(os.path.join(output_dir, f'offloading_stats_vehicle_{v_id}.png'))
        plt.close()

def plot_queue_size_cdf_per_vehicle(df, output_dir):
    """
    Gera a CDF do tamanho da fila por veículo.
    """
    data = df[df['Metric'] == 'QueueSize_Decision'].copy()
    if data.empty: return

    vehicles = data['EntityID'].unique()
    for v_id in vehicles:
        v_data = data[data['EntityID'] == v_id]
        if v_data.empty: continue

        plt.figure()
        sns.ecdfplot(data=v_data, x='Value', linewidth=2, color='purple')
        
        plt.title(f'CDF de Tamanho da Fila - Veículo {v_id}')
        plt.xlabel('Tamanho da Fila (Tarefas)')
        plt.ylabel('Probabilidade P(X <= x)')
        plt.grid(True, which='both', linestyle='--', alpha=0.5)
        plt.tight_layout()
        plt.savefig(os.path.join(output_dir, f'cdf_queue_size_vehicle_{v_id}.png'))
        plt.close()

def plot_deadline_margin_cdf_per_vehicle(df, output_dir):
    """
    Gera a CDF da margem do deadline (Deadline - Latency) por veículo.
    Positivo = Slack (Abaixo do deadline).
    Negativo = Violação (Acima do deadline).
    """
    data = df[df['Metric'] == 'TaskMargin'].copy()
    if data.empty: return

    vehicles = data['EntityID'].unique()
    for v_id in vehicles:
        v_data = data[data['EntityID'] == v_id]
        if v_data.empty: continue

        plt.figure()
        sns.ecdfplot(data=v_data, x='Value', linewidth=2, color='orange')
        
        plt.axvline(0, color='red', linestyle='--', label='Deadline (0s)')
        
        plt.title(f'CDF de Margem do Deadline (Slack) - Veículo {v_id}')
        plt.xlabel('Margem (s) [Positivo=Bom, Negativo=Ruim]')
        plt.ylabel('Probabilidade P(X <= x)')
        plt.legend(loc='lower right')
        plt.grid(True, which='both', linestyle='--', alpha=0.5)
        plt.tight_layout()
        plt.savefig(os.path.join(output_dir, f'cdf_deadline_margin_vehicle_{v_id}.png'))
        plt.close()

def plot_deadline_margin_global(df, output_dir):
    """
    Gera a CDF da margem do deadline globalmente.
    """
    data = df[df['Metric'] == 'TaskMargin'].copy()
    if data.empty: return

    plt.figure()
    sns.ecdfplot(data=data, x='Value', hue='EntityID', linewidth=2, palette='tab10')
    
    plt.axvline(0, color='red', linestyle='--', label='Deadline')
    
    plt.title('CDF Global de Margem do Deadline')
    plt.xlabel('Margem (s) [Positivo=Bom, Negativo=Ruim]')
    plt.ylabel('Probabilidade P(X <= x)')
    plt.legend(title='Veículo')
    plt.grid(True, which='both', linestyle='--', alpha=0.5)
    plt.tight_layout()
    plt.savefig(os.path.join(output_dir, 'cdf_deadline_margin_global.png'))
    plt.close()

def main():
    parser = argparse.ArgumentParser(description='Plot Results Per Vehicle (Original Style)')
    parser.add_argument('--input', type=str, default='build/Desktop-Debug/results/experiment_001.csv',
                        help='Input CSV file')
    parser.add_argument('--output', type=str, default='build/Desktop-Debug/results/',
                        help='Output directory')
    
    args = parser.parse_args()
    
    if not os.path.exists(args.output):
        os.makedirs(args.output)
        
    setup_plot_style()
    df = load_data(args.input)
    
    if df is not None:
        print("Gerando gráficos detalhados por veículo (Estilo Original)...")
        
        # Otimização (from original script)
        df['Metric'] = df['Metric'].astype('category')
        if 'Tag' in df.columns:
            df['Tag'] = df['Tag'].astype('category')

        plot_latency_cdf_per_vehicle(df, args.output)
        plot_queue_dynamics_per_vehicle(df, args.output)
        plot_queue_size_cdf_per_vehicle(df, args.output)
        plot_deadline_margin_cdf_per_vehicle(df, args.output)
        plot_deadline_margin_global(df, args.output)
        plot_success_rate_per_vehicle(df, args.output)
        plot_offloading_stats_per_vehicle(df, args.output)
        
        print(f"Gráficos salvos em: {args.output}")

if __name__ == "__main__":
    main()
