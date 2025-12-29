#ifndef METRIC_H
#define METRIC_H

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <memory>
#include <mutex>
#include <map>

// Estrutura de dados pura para uma linha do CSV
struct MetricRecord {
    double time;
    int entity_id;
    std::string metric_name;
    double value;
    // Opcional: Tags extras (ex: "Local", "Offload")
    std::string tag;
};

// Interface do Listener (O "Contrato")
class IMetricListener {
public:
    virtual ~IMetricListener() = default;
    virtual void onMetricRecorded(const MetricRecord& record) = 0;
};

// O Coletor Concreto (Gera o CSV)
class CSVMetricsCollector : public IMetricListener {
    std::string filename;
    std::vector<MetricRecord> buffer;
    std::mutex mtx;
    const size_t BUFFER_LIMIT = 10000; // Flush a cada 10k registros para performance

public:
    CSVMetricsCollector(const std::string& fname) : filename(fname) {
        std::ofstream file(filename, std::ios::out | std::ios::trunc);
        file << "Time,EntityID,Metric,Value,Tag\n"; // Header CSV
    }

    ~CSVMetricsCollector() {
        flush();
    }

    void onMetricRecorded(const MetricRecord& record) override {
        std::lock_guard<std::mutex> lock(mtx);
        buffer.push_back(record);
        if (buffer.size() >= BUFFER_LIMIT) {
            flush();
        }
    }

    void flush() {
        if (buffer.empty()) return;
        std::ofstream file(filename, std::ios::out | std::ios::app);
        for (const auto& rec : buffer) {
            file << rec.time << ","
                 << rec.entity_id << ","
                 << rec.metric_name << ","
                 << rec.value << ","
                 << rec.tag << "\n";
        }
        buffer.clear();
    }
};

// Hub Global (Singleton) para despachar métricas
class MetricsHub {
    std::vector<std::shared_ptr<IMetricListener>> listeners;

public:
    static MetricsHub& instance() {
        static MetricsHub hub;
        return hub;
    }

    void addListener(std::shared_ptr<IMetricListener> listener) {
        listeners.push_back(listener);
    }

    void record(double time, int entity_id, const std::string& name, double value, const std::string& tag = "") {
        MetricRecord rec{time, entity_id, name, value, tag};
        for (auto& l : listeners) {
            l->onMetricRecorded(rec);
        }
    }
};

// Macro para facilitar o uso no código (S syntactic sugar)
#define RECORD_METRIC(sim, entity_id, name, val, tag) \
MetricsHub::instance().record(sim.now(), entity_id, name, val, tag)

#endif // METRIC_H
