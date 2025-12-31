# Global Chaos Implementation - Documentação

## Descrição

Este documento descreve a implementação do **Global Chaos State** no simulador, seguindo o modelo matemático de um processo AR(1) para representar incerteza ambiental temporalmente correlacionada.

## Problema Anterior

O código anterior tinha as seguintes falhas:

1. **Cada variável chamando RNG independentemente**: `pdrift(size)`, `pdrift(density)`, `pdrift(deadline)` - cada um com seu próprio ruído
2. **Sem correlação estrutural**: Os parâmetros eram tratados como se fossem independentes
3. **Ruído branco, não caos**: Sem memória temporal entre chamadas

```cpp
// ❌ ANTES (errado):
double min_s = Rng::pdrift(Config::TASK_MIN_SIZE);  // RNG próprio
double max_s = Rng::pdrift(Config::TASK_MAX_SIZE);  // RNG próprio diferente
```

## Solução Implementada

### 1. ChaosManager (Singleton)

Novo arquivo `core/ChaosManager.h` que implementa:

```cpp
class ChaosManager {
    // Processo AR(1): z_t = ρ * z_{t-1} + σ * ε_t
    static constexpr double RHO = 0.9;  // Memória
    double z = 0.0;  // Estado de caos
    
    void update();           // Atualiza o estado
    double apply_drift(double base);  // Aplica drift ao parâmetro
    double get_state();      // Retorna z_t atual
};
```

### 2. Uso Correto do Drift

Agora todas as variáveis compartilham o **mesmo estado de caos** no mesmo instante:

```cpp
// ✅ DEPOIS (correto):
if (Config::FIELD_TOTAL_CHAOS) {
    ChaosManager::instance().update();  // Atualiza UMA VEZ
}

double drift = ChaosManager::instance().get_state();  // MESMO drift para todos

double min_s = Rng::pdrift(Config::TASK_MIN_SIZE, drift);
double max_s = Rng::pdrift(Config::TASK_MAX_SIZE, drift);
double min_d = Rng::pdrift(Config::TASK_MIN_DEADLINE, drift);
double max_d = Rng::pdrift(Config::TASK_MAX_DEADLINE, drift);
```

## Arquivos Modificados

| Arquivo | Mudança |
|---------|---------|
| `core/ChaosManager.h` | **NOVO** - Implementa o gerenciador de caos global |
| `core/Config.h` | Removido include circular de Rng.h |
| `utils/Rng.h` | Removido ChaosState e update_chaos (movidos para ChaosManager) |
| `model/Task.cpp` | Usa ChaosManager para drift correlacionado |
| `model/CPU.cpp` | Usa ChaosManager para drift de processing time |
| `model/RandomPolicy.cpp` | Usa ChaosManager para drift de decision time |
| `model/FirstRemotePolicy.cpp` | Usa ChaosManager para drift de decision time |
| `model/OffPolicy.cpp` | Usa ChaosManager para drift de decision time |
| `model/IntelligentPolicy.cpp` | Usa ChaosManager para drift de decision time |
| `core/EnergyManager.h` | Usa ChaosManager para drift de energia |
| `events/TaskGenerationEvent.cpp` | Usa ChaosManager para drift de inter-arrival |
| `main.cpp` | Inicializa ChaosManager com seed |

## Modelo Matemático

### Processo AR(1)
$$z_t = \rho z_{t-1} + \sigma \varepsilon_t$$

Onde:
- $\rho = 0.9$ → memória/persistência
- $\sigma$ = `Config::CHAOS_INTENSITY` (default: 2.5)
- $\varepsilon_t \sim \mathcal{U}(-1, 1)$

### Função de Drift
$$\text{value}' = \max(\text{value} \times (1 + z_t), \epsilon)$$

## Propriedades do Novo Sistema

✅ **Correlação temporal**: O caos de hoje depende do caos de ontem
✅ **Correlação estrutural**: Todos os parâmetros compartilham o mesmo caos
✅ **Controlável**: Intensidade ajustável via `CHAOS_INTENSITY`
✅ **Reproduzível**: Seed do ChaosManager sincronizado com seed global

## Uso

```bash
# Sem caos (modo estacionário)
./tank_simulator Local 100 1978

# Com caos (modo não-estacionário)
./tank_simulator Local 100 1978 --chaos
```

## Definição Formal (para artigo)

> *We model environmental uncertainty as a latent global chaos state shared across all task and network parameters, evolving as a temporally correlated stochastic process.*

## Próximos Passos

1. Adicionar logging do estado de caos `z_t` ao longo do tempo
2. Implementar eventos de shock (picos raros de alta intensidade)
3. Medir Lyapunov exponent discreto para caracterização formal
4. Integrar com RL como estado oculto do ambiente
