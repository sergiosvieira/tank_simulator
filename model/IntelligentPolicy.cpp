#include "IntelligentPolicy.h"
#include "Model.h"

DecisionResult IntelligentPolicy::decide(Task::PtrTask task, std::vector<RSU::PtrRSU>& rsus) {
    // 1. Proteção contra Crash (Safety Check)
    if (!host) return {DecisionType::Local, nullptr};

    // --- ANÁLISE DO ESTADO LOCAL (Premissa 1: Sobrevivência do Buffer) ---
    size_t q_curr = host->get_current_queue_size();
    size_t q_max = host->get_max_queue_size();

    // Regra Crítica: Se a fila tem 2 ou mais itens (limite é 3), OFFLOAD IMEDIATO.
    // Isso evita o 'FullQueueError' que causou 61% de falha na Local.
    if (q_curr >= (q_max - 1)) {
        if (rsus.empty()) return {DecisionType::Local, nullptr}; // Sem opção

        // Seleção Inteligente de RSU (Best-Fit Frequency)
        // Em vez de aleatório, pega o RSU mais potente para compensar atraso de rede
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

    // --- ESTIMATIVA DE DEADLINE (Premissa 2: Viabilidade de Tempo) ---
    // T_local = Ciclos / Frequencia_Local
    double local_proc_time = task->total_cycles() / host->cpu.get_freq();

    // Se não vai dar tempo de processar localmente antes do deadline...
    if (local_proc_time > task->get_deadline()) {
        if (!rsus.empty()) {
            // Tenta salvar a tarefa mandando para fora
            return {DecisionType::Remote, rsus[0]};
        }
    }

    // --- CONSERVAÇÃO DE ENERGIA (Premissa 3: Custo Mínimo) ---
    // Se o buffer está seguro e temos tempo, ficamos LOCAL para economizar energia de Tx.
    return {DecisionType::Local, nullptr};
}

double IntelligentPolicy::decision_time(Task::PtrTask task) {
    // Custo ligeiramente maior que Random pois faz cálculos (1ms a mais simulado)
    return Rng::uniform(0.004, 0.006);
}
