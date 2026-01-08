#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <future>
#include <iomanip>
#include <iostream>
#include <map>
#include <mutex>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

// Namespace for cleaner code
using namespace std;
namespace fs = filesystem;

// --- ESTRUTURAS DE DADOS ---

struct SimulationStats {
  string policy;
  string filename;

  // Contadores
  long total_tasks = 0;
  long successes = 0;
  long failures = 0; // Overflow ou outros
  long offload_local = 0;
  long offload_remote = 0;

  // Energia
  double energy_cpu = 0.0;
  double energy_tx = 0.0;

  // Latência (armazenamos tudo para calcular percentis ou histograma depois)
  vector<double> latencies;
  vector<double> transfer_times; // New metric

  // Time Series (Binning)
  // Map: Time (int) -> pair<sum, count>
  map<int, pair<double, int>> queue_series;      // Decision queue
  map<int, pair<double, int>> queue_proc_series; // Processing queue
  map<int, pair<double, int>> battery_series;
  map<int, int> failures_series; // Time -> count
};

// --- CONFIGURAÇÃO ---
const int TIME_BIN_SIZE = 1; // 1 segundo por bin

// --- FUNÇÃO DE PARSER (Uma por arquivo) ---
SimulationStats parse_file(const fs::path &path) {
  SimulationStats stats;
  stats.filename = path.filename().string();

  // Extrair policy do nome do arquivo (ex: experiment_001_intelligent_123.csv)
  string name = stats.filename;
  if (name.find("local") != string::npos)
    stats.policy = "Local";
  else if (name.find("random") != string::npos)
    stats.policy = "Random";
  else if (name.find("intelligent") != string::npos)
    stats.policy = "Intelligent";
  else if (name.find("first_remote") != string::npos)
    stats.policy = "FirstRemote";
  else
    stats.policy = "Unknown";

  ifstream file(path);
  if (!file.is_open())
    return stats;

  string line;
  // Ignorar header se existir
  getline(file, line);
  if (line.find("Time") == string::npos) {
    // Se a primeira linha não for header, resetar
    file.clear();
    file.seekg(0);
  }

  // Buffer reutilizável
  double time_val, value_val;
  string segment, entity, metric, val_str, tag, dummy;

  while (getline(file, line)) {
    if (line.empty())
      continue;

    stringstream ss(line);
    // Formato: Time,EntityID,Metric,Value,Tag,TaskID,Location
    // Ex: 1.02, 1, TaskSuccess, 1, , 5, ...

    // Leitura otimizada (getline é seguro)
    string s_time, s_entity, s_metric, s_value, s_tag;

    getline(ss, s_time, ',');
    getline(ss, s_entity, ',');
    getline(ss, s_metric, ',');
    getline(ss, s_value, ',');
    getline(ss, s_tag, ',');

    try {
      time_val = stod(s_time);

      // FILTROS E PARSING
      if (s_metric == "TaskSuccess") {
        // Value pode ser 1 ou 0
        if (stod(s_value) > 0.5)
          stats.successes++;
        stats.total_tasks++;
      } else if (s_metric == "FullQueueError") {
        stats.failures++;
        int bin = (int)time_val;
        stats.failures_series[bin]++;
        stats.total_tasks++; // Conta como task tentada
      } else if (s_metric == "TaskLatency") {
        stats.latencies.push_back(stod(s_value));
      } else if (s_metric == "TransferTime") {
        stats.transfer_times.push_back(stod(s_value));
      } else if (s_metric == "EnergyConsumption") {
        double e = stod(s_value);
        if (s_tag.find("CpuOnly") != string::npos)
          stats.energy_cpu += e;
        else if (s_tag.find("TxOnly") != string::npos)
          stats.energy_tx += e;
      } else if (s_metric == "QueueSize_Decision") {
        int bin = (int)time_val;
        stats.queue_series[bin].first += stod(s_value);
        stats.queue_series[bin].second++;
      } else if (s_metric == "QueueSize_Processing") {
        int bin = (int)time_val;
        stats.queue_proc_series[bin].first += stod(s_value);
        stats.queue_proc_series[bin].second++;
      } else if (s_metric == "BatteryRemaining") {
        int bin = (int)time_val;
        stats.battery_series[bin].first += stod(s_value);
        stats.battery_series[bin].second++;
      } else if (s_metric == "OffloadingType") {
        if (s_value == "1")
          stats.offload_remote++;
        else
          stats.offload_local++;
      }

    } catch (...) {
      continue;
    }
  }

  return stats;
}

int main(int argc, char *argv[]) {
  // 1. Identificar arquivos
  string results_dir = "results/";
  if (argc > 1)
    results_dir = argv[1];

  vector<fs::path> files;
  try {
    for (const auto &entry : fs::directory_iterator(results_dir)) {
      if (entry.path().extension() == ".csv" &&
          entry.path().string().find("summary") == string::npos &&
          entry.path().string().find("metrics") == string::npos &&
          entry.path().string().find("aggregated") == string::npos) {
        files.push_back(entry.path());
      }
    }
  } catch (...) {
    cerr << "Erro ao abrir diretorio " << results_dir << endl;
    return 1;
  }

  cout << "Encontrados " << files.size()
       << " arquivos. Iniciando processamento paralelo..." << endl;

  // 2. Processamento Paralelo
  vector<future<SimulationStats>> futures;
  for (const auto &f : files) {
    futures.push_back(async(launch::async, parse_file, f));
  }

  // 3. Agregar Resultados Globalmente
  // Vamos criar dois arquivos de saida:
  // a) aggregated_summary.csv (Uma linha por arquivo/seed)
  // b) aggregated_timeseries.csv (Binado por tempo e policy)

  ofstream out_summary("results/aggregated_summary.csv");
  out_summary << "Policy,Filename,SuccessRate,TotalEnergyCPU,TotalEnergyTx,"
                 "AvgLatency,P50Latency,P95Latency,Failures,OffloadLocal,"
                 "OffloadRemote,AvgTransferTime"
              << endl;

  // Para timeseries, precisamos agregar por policy primeiro
  // Policy -> TimeBin -> {SumValue, Count} (para tirar média global)
  map<string, map<int, pair<double, int>>> policy_queue_agg;
  map<string, map<int, pair<double, int>>> policy_queue_proc_agg;
  map<string, map<int, pair<double, int>>> policy_batt_agg;

  // Para latências globais (para boxplot) - vamos samplear para não explodir
  // Ou podemos salvar percentis per file no summary e tirar a média dos
  // percentis (aproximação aceitável)

  int processed = 0;
  for (auto &fut : futures) {
    SimulationStats stats = fut.get();
    if (stats.policy == "Unknown")
      continue;

    // Calcular estatísticas locais
    double succ_rate = stats.total_tasks > 0
                           ? (double)stats.successes / stats.total_tasks
                           : 0.0;

    // Latencia
    double lat_avg = 0, lat_p50 = 0, lat_p95 = 0;
    if (!stats.latencies.empty()) {
      sort(stats.latencies.begin(), stats.latencies.end());
      double sum =
          accumulate(stats.latencies.begin(), stats.latencies.end(), 0.0);
      lat_avg = sum / stats.latencies.size();
      lat_p50 = stats.latencies[stats.latencies.size() * 0.50];
      lat_p95 = stats.latencies[stats.latencies.size() * 0.95];
    }

    double transfer_avg = 0;
    if (!stats.transfer_times.empty()) {
      double sum = accumulate(stats.transfer_times.begin(),
                              stats.transfer_times.end(), 0.0);
      transfer_avg = sum / stats.transfer_times.size();
    }

    out_summary << stats.policy << "," << stats.filename << "," << succ_rate
                << "," << stats.energy_cpu << "," << stats.energy_tx << ","
                << lat_avg << "," << lat_p50 << "," << lat_p95 << ","
                << stats.failures << "," << stats.offload_local << ","
                << stats.offload_remote << "," << transfer_avg << endl;

    // Agregar TimeSeries
    for (auto const &[bin, val] : stats.queue_series) {
      policy_queue_agg[stats.policy][bin].first += val.first; // Soma das somas
      policy_queue_agg[stats.policy][bin].second +=
          val.second; // Soma dos counts
    }
    for (auto const &[bin, val] : stats.queue_proc_series) {
      policy_queue_proc_agg[stats.policy][bin].first += val.first;
      policy_queue_proc_agg[stats.policy][bin].second += val.second;
    }
    for (auto const &[bin, val] : stats.battery_series) {
      policy_batt_agg[stats.policy][bin].first += val.first;
      policy_batt_agg[stats.policy][bin].second += val.second;
    }

    processed++;
    if (processed % 10 == 0)
      cout << "\rProcessado: " << processed << "/" << files.size() << flush;
  }
  cout << endl << "Gerando TimeSeries..." << endl;

  ofstream out_ts("results/aggregated_timeseries.csv");
  out_ts << "Policy,Time,AvgQueueSize,AvgQueueProc,AvgBattery" << endl;

  // Iterar por policies e bins
  for (auto const &[policy, bin_map] : policy_queue_agg) {
    // Encontrar range de tempo
    int max_time = bin_map.rbegin()->first;
    for (int t = 0; t <= max_time; t++) {
      double avg_q = 0, avg_qp = 0, avg_b = 0;

      // Decision Queue
      if (bin_map.count(t)) {
        auto &p = bin_map.at(t);
        if (p.second > 0)
          avg_q = p.first / p.second;
      }

      // Processing Queue
      if (policy_queue_proc_agg[policy].count(t)) {
        auto &p = policy_queue_proc_agg[policy].at(t);
        if (p.second > 0)
          avg_qp = p.first / p.second;
      }

      // Battery
      if (policy_batt_agg[policy].count(t)) {
        auto &p = policy_batt_agg[policy].at(t);
        if (p.second > 0)
          avg_b = p.first / p.second;
      }

      out_ts << policy << "," << t << "," << avg_q << "," << avg_qp << ","
             << avg_b << endl;
    }
  }

  cout << "Concluido. Arquivos gerados:" << endl;
  cout << "  - results/aggregated_summary.csv" << endl;
  cout << "  - results/aggregated_timeseries.csv" << endl;

  return 0;
}
