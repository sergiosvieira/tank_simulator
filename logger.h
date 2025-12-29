#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <fstream>
#include <string>
#include <mutex>
#include <sstream>
#include <filesystem>
#include <iomanip>

#if defined(_MSC_VER)
#define __METHOD_NAME__ __FUNCSIG__
#elif defined(__GNUC__) || defined(__clang__)
#define __METHOD_NAME__ __PRETTY_FUNCTION__
#else
#define __METHOD_NAME__ __func__
#endif

// Níveis de Log
enum class LogLevel { DEBUG, INFO, WARN, ERROR, DATA };

class Logger {
private:
    std::ofstream traceFile;
    std::ofstream metricsFile;
    std::mutex mtx; // Garante thread-safety (caso expanda para multithread no futuro)
    bool debugEnabled = true;

    // Construtor privado (Singleton)
    Logger() {
        // Cria diretório de logs se não existir (C++17)
        std::filesystem::create_directory("logs");
        traceFile.open("logs/simulation_trace.log", std::ios::out | std::ios::trunc);
        // Header do arquivo de métricas (CSV)
        std::filesystem::create_directory("logs");
        metricsFile.open("results/simulation_metrics.csv", std::ios::out | std::ios::trunc);
        metricsFile << "Time,EntityID,MetricType,Value,Extra\n";
    }

    ~Logger() {
        if (traceFile.is_open()) traceFile.close();
        if (metricsFile.is_open()) metricsFile.close();
    }

public:
    // Acesso à instância única
    static Logger& instance() {
        static Logger instance;
        return instance;
    }

    // Desabilitar DEBUG para rodar simulações pesadas mais rápido
    void setDebug(bool enable) { debugEnabled = enable; }

    // 1. LOG DE RASTREIO (Humano)
    void log(double simTime, LogLevel level, const std::string& msg) {
        if (level == LogLevel::DEBUG && !debugEnabled) return;

        std::lock_guard<std::mutex> lock(mtx);

        // Formato: [TIME] [LEVEL] Message
        traceFile << std::fixed << std::setprecision(4) << "[" << simTime << "] ";

        switch(level) {
        case LogLevel::DEBUG: traceFile << "[DEBUG] "; break;
        case LogLevel::INFO:  traceFile << "[INFO]  "; break;
        case LogLevel::WARN:  traceFile << "[WARN]  "; break;
        case LogLevel::ERROR: traceFile << "[ERROR] "; break;
        default: break;
        }
        traceFile << msg
                  << std::endl;

        // Opcional: Espelhar erros críticos no console
        if (level == LogLevel::ERROR) {
            std::cerr << "\033[1;31m[ERROR] " << msg << "\033[0m" << std::endl;
        }
    }

    // 2. LOG DE MÉTRICAS (Máquina/Gráficos)
    // Grava diretamente em CSV: Time, EntityID, MetricName, Value
    void recordMetric(double simTime, int entityId, const std::string& metricName, double value, const std::string& extra = "") {
        std::lock_guard<std::mutex> lock(mtx);
        metricsFile << std::fixed << std::setprecision(6)
                    << simTime << ","
                    << entityId << ","
                    << metricName << ","
                    << value << ","
                    << extra << "\n";
    }
};

// ********** Macros para Facilidade de Uso **********
// Isso captura a instância do Simulador para pegar o tempo automaticamente se disponível,
// ou você pode passar o tempo explicitamente.
// Para este exemplo, assumimos passagem explícita do tempo 't'.

#define LOG_INFO(t, msg) Logger::instance().log(t, LogLevel::INFO, msg)
#define LOG_DEBUG(t, msg) Logger::instance().log(t, LogLevel::DEBUG, msg)
#define LOG_ERROR(t, msg) Logger::instance().log(t, LogLevel::ERROR, msg)

// Macro para registrar dados estatísticos
#define RECORD_METRIC(t, id, name, val) Logger::instance().recordMetric(t, id, name, val)

#endif // LOGGER_H
