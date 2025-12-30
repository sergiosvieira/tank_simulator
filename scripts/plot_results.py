import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import argparse
import os

def load_data(filepath):
    """Carrega o CSV e retorna um DataFrame limpo."""
    if not os.path.exists(filepath):
        print(f"Erro: Arquivo não encontrado em {filepath}")
        return None
    
    df = pd.read_csv(filepath)
    return df

def plot_global_latency(df, output_dir):
    """Plota a distribuição de latência global e latência ao longo do tempo."""
    latency_df = df[df['Metric'] == 'TaskLatency']
    
    if latency_df.empty:
        print("Aviso: Sem dados de TaskLatency.")
        return

    plt.figure(figsize=(10, 6))
    sns.histplot(data=latency_df, x='Value', kde=True, bins=30)
    plt.title('Distribuição Global de Latência')
    plt.xlabel('Latência (s)')
    plt.ylabel('Contagem')
    plt.savefig(os.path.join(output_dir, 'global_latency_dist.png'))
    plt.close()

    plt.figure(figsize=(12, 6))
    sns.scatterplot(data=latency_df, x='Time', y='Value', alpha=0.6, label='Latência Instantânea')
    # Média móvel
    latency_df = latency_df.sort_values('Time')
    latency_df['RollingMean'] = latency_df['Value'].rolling(window=50, min_periods=1).mean()
    plt.plot(latency_df['Time'], latency_df['RollingMean'], color='red', linewidth=2, label='Média Móvel (50 tasks)')
    
    plt.title('Latência ao Longo do Tempo')
    plt.xlabel('Tempo de Simulação (s)')
    plt.ylabel('Latência (s)')
    plt.legend()
    plt.savefig(os.path.join(output_dir, 'latency_over_time.png'))
    plt.close()

def plot_success_rate(df, output_dir):
    """Plota a taxa de sucesso global."""
    success_df = df[df['Metric'] == 'TaskSuccess']
    if success_df.empty:
        return

    success_counts = success_df['Value'].value_counts()
    # Value 1.0 = Sucesso, 0.0 = Falha
    labels = ['Sucesso', 'Falha']
    # Mapear counts para garantir ordem
    counts = [success_counts.get(1.0, 0), success_counts.get(0.0, 0)]
    
    plt.figure(figsize=(6, 6))
    plt.pie(counts, labels=labels, autopct='%1.1f%%', colors=['#66b3ff','#ff9999'], startangle=90)
    plt.title('Taxa Global de Sucesso das Tarefas')
    plt.savefig(os.path.join(output_dir, 'success_rate.png'))
    plt.close()

def plot_offloading_stats(df, output_dir):
    """Plota estatísticas de offloading (Local vs Remote)."""
    offload_df = df[df['Metric'] == 'OffloadingType']
    if offload_df.empty:
        return

    # Usar a coluna 'Tag' que contém "Local" ou "Remote"
    counts = offload_df['Tag'].value_counts()
    
    plt.figure(figsize=(6, 6))
    counts.plot.pie(autopct='%1.1f%%', startangle=90, colors=['#99ff99','#ffcc99'])
    plt.title('Proporção de Offloading (Local vs Remoto)')
    plt.ylabel('')
    plt.savefig(os.path.join(output_dir, 'offloading_ratio.png'))
    plt.close()

def plot_energy_consumption(df, output_dir):
    """Plota consumo de energia acumulado."""
    energy_df = df[df['Metric'] == 'EnergyConsumption']
    if energy_df.empty:
        return

    energy_df = energy_df.sort_values('Time')
    energy_df['Accumulated'] = energy_df['Value'].cumsum()

    plt.figure(figsize=(10, 6))
    plt.plot(energy_df['Time'], energy_df['Accumulated'], color='green')
    plt.fill_between(energy_df['Time'], energy_df['Accumulated'], color='green', alpha=0.3)
    plt.title('Consumo Acumulado de Energia')
    plt.xlabel('Tempo (s)')
    plt.ylabel('Energia (J)')
    plt.savefig(os.path.join(output_dir, 'energy_consumption.png'))
    plt.close()

def main():
    parser = argparse.ArgumentParser(description='Plot Simulation Results')
    parser.add_argument('--input', type=str, default='build/Desktop-Debug/results/experiment_001.csv',
                        help='Caminho para o arquivo CSV de métricas')
    parser.add_argument('--output', type=str, default='build/Desktop-Debug/results/',
                        help='Diretório para salvar os gráficos')
    
    args = parser.parse_args()
    
    if not os.path.exists(args.output):
        os.makedirs(args.output)
        
    print(f"Lendo dados de: {args.input}")
    df = load_data(args.input)
    
    if df is not None:
        print("Gerando gráficos...")
        # Configurar estilo
        sns.set_theme(style="whitegrid")
        
        plot_global_latency(df, args.output)
        plot_success_rate(df, args.output)
        plot_offloading_stats(df, args.output)
        plot_energy_consumption(df, args.output)
        
        print(f"Gráficos salvos em: {args.output}")

if __name__ == "__main__":
    main()
