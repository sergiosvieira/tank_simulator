#include "RandomPolicy.h"

#include "../core/ChaosManager.h"
#include "Model.h"

DecisionResult RandomPolicy::decide(Task::PtrTask task,
                                    std::vector<RSU::PtrRSU> &rsus) {
  DecisionType dt = DecisionType::Local;
  RSU::PtrRSU dst = nullptr;
  if (rsus.size() == 0) return {dt, dst};
  if (Rng::uniform(0.0, 1.0) >= 0.5) {
    dt = DecisionType::Remote;
    dst = Rng::sample<RSU::PtrRSU>(rsus);
  }
  return {dt, dst};
}

double RandomPolicy::decision_time(Task::PtrTask task) {
  double result = Rng::uniform(0.003, 0.005);  // 3–5 ms
  if (Config::FIELD_TOTAL_CHAOS) {
    double drift = ChaosManager::instance().get_state();
    return Rng::pdrift(result, drift);
  }
  return result;
}
