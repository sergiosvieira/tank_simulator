import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import argparse
import os

def load_data(filepath):
    if not os.path.exists(filepath):
        print(f"Erro: Arquivo não encontrado em {filepath}")
        return None
    return pd.read_csv(filepath)

def plot_latency_comparison(df, output_dir):
    """Boxplot of TaskLatency per EntityID (Policy comparison)."""
    data = df[df['Metric'] == 'TaskLatency']
    if data.empty:
        return

    plt.figure(figsize=(10, 6))
    sns.boxplot(data=data, x='EntityID', y='Value', hue='EntityID', palette='viridis', legend=False)
    plt.title('Comparação de Latência por Veículo (Política)')
    plt.ylabel('Latência (s)')
    plt.xlabel('ID do Veículo')
    plt.savefig(os.path.join(output_dir, 'comparison_latency.png'))
    plt.close()

def plot_success_rate_comparison(df, output_dir):
    """Bar plot of Success Rate per EntityID."""
    data = df[df['Metric'] == 'TaskSuccess']
    if data.empty:
        return

    # Group by EntityID and Value (0.0 or 1.0)
    grouped = data.groupby(['EntityID', 'Value']).size().unstack(fill_value=0)
    
    # Calculate percentage
    if 1.0 not in grouped.columns:
        grouped[1.0] = 0
    if 0.0 not in grouped.columns:
        grouped[0.0] = 0
        
    total = grouped[1.0] + grouped[0.0]
    success_rate = (grouped[1.0] / total) * 100
    
    plt.figure(figsize=(10, 6))
    sns.barplot(x=success_rate.index, y=success_rate.values, hue=success_rate.index, palette='viridis', legend=False)
    plt.title('Taxa de Sucesso por Veículo')
    plt.ylabel('Sucesso (%)')
    plt.xlabel('ID do Veículo')
    plt.ylim(0, 100)
    plt.savefig(os.path.join(output_dir, 'comparison_success_rate.png'))
    plt.close()

def plot_energy_comparison(df, output_dir):
    """Bar plot of Total Energy Consumption per EntityID."""
    data = df[df['Metric'] == 'EnergyConsumption']
    if data.empty:
        return

    total_energy = data.groupby('EntityID')['Value'].sum()

    plt.figure(figsize=(10, 6))
    sns.barplot(x=total_energy.index, y=total_energy.values, hue=total_energy.index, palette='magma', legend=False)
    plt.title('Consumo Total de Energia por Veículo')
    plt.ylabel('Energia Total (J)')
    plt.xlabel('ID do Veículo')
    plt.savefig(os.path.join(output_dir, 'comparison_energy.png'))
    plt.close()

def plot_offloading_distribution(df, output_dir):
    """Stacked bar plot of Offloading Type (Local vs Remote) per EntityID."""
    data = df[df['Metric'] == 'OffloadingType']
    if data.empty:
        return

    # Tag contains 'Local' or 'Remote'
    # Count occurrences
    grouped = data.groupby(['EntityID', 'Tag']).size().unstack(fill_value=0)
    
    # Normalize to 100%
    normalized = grouped.div(grouped.sum(axis=1), axis=0) * 100
    
    normalized.plot(kind='bar', stacked=True, figsize=(10, 6), colormap='coolwarm')
    plt.title('Distribuição de Offloading (Local vs Remoto)')
    plt.ylabel('Porcentagem (%)')
    plt.xlabel('ID do Veículo')
    plt.legend(title='Tipo de Processamento')
    plt.savefig(os.path.join(output_dir, 'comparison_offloading_dist.png'))
    plt.close()

def main():
    parser = argparse.ArgumentParser(description='Compare Offloading Policies')
    parser.add_argument('--input', type=str, default='build/Desktop-Debug/results/experiment_001.csv',
                        help='Input CSV file')
    parser.add_argument('--output', type=str, default='build/Desktop-Debug/results/',
                        help='Output directory')
    
    args = parser.parse_args()
    
    if not os.path.exists(args.output):
        os.makedirs(args.output)

    df = load_data(args.input)
    if df is not None:
        print("Gerando gráficos comparativos...")
        sns.set_theme(style="whitegrid")
        
        plot_latency_comparison(df, args.output)
        plot_success_rate_comparison(df, args.output)
        plot_energy_comparison(df, args.output)
        plot_offloading_distribution(df, args.output)
        
        print(f"Gráficos salvos em: {args.output}")

if __name__ == "__main__":
    main()
