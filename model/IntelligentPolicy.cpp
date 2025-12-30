#include "IntelligentPolicy.h"
#include "Model.h"

DecisionResult IntelligentPolicy::decide(Task::PtrTask task, std::vector<RSU::PtrRSU>& rsus) {
    if (!host) return {DecisionType::Local, nullptr};

    size_t q_curr = host->get_current_queue_size();

    // 1. Estimativa Realista de Tempo Local (Wait Time + Service Time)
    // Assumindo que tasks na fila têm tamanho médio similar ou somando seus ciclos se acessível
    double service_time = task->total_cycles() / host->cpu.get_freq();

    // Estima quanto tempo a fila atual vai levar para esvaziar
    // Se você não tem acesso aos ciclos das tasks na fila, use uma média
    double wait_time = q_curr * service_time;

    double total_local_time = wait_time + service_time;

    // 2. Decisão Baseada em Deadline (Agora considerando a espera)
    // Adicionamos uma margem de segurança (ex: 10%)
    if (total_local_time > (task->get_deadline() * 0.90)) {
        if (!rsus.empty()) {
            // Lógica Best-Fit (Reutilizada para ambos os casos)
            RSU::PtrRSU best_rsu = nullptr;
            double max_freq = -1.0;
            for (auto& rsu : rsus) {
                if (rsu->cpu.get_freq() > max_freq) {
                    max_freq = rsu->cpu.get_freq();
                    best_rsu = rsu;
                }
            }
            return {DecisionType::Remote, best_rsu};
        }
    }

    // 3. Balanceamento de Carga Preventivo (Load Balancing)
    // Não espere a fila encher. Se tiver mais de 1 item, considere offload se tiver RSU livre.
    // Isso melhora a latência média (Gráfico B)
    if (q_curr > 1 && !rsus.empty()) {
        return {DecisionType::Remote, rsus[0]}; // Ou Random, ou Best-Fit
    }

    return {DecisionType::Local, nullptr};
}

double IntelligentPolicy::decision_time(Task::PtrTask task) {
    // Custo ligeiramente maior que Random pois faz cálculos (1ms a mais simulado)
    return Rng::uniform(0.004, 0.006);
}
