#include "OffPolicy.h"
#include "../core/ChaosManager.h"
#include "model/RSU.h"

DecisionResult OffPolicy::decide(Task::PtrTask task,
                                 std::vector<RSU::PtrRSU> &rsus) {
  return {DecisionType::Local, nullptr};
}

double OffPolicy::decision_time(Task::PtrTask task) {
  double result = Rng::uniform(0.003, 0.005); // 3–5 ms
  if (Config::FIELD_TOTAL_CHAOS) {
    double drift = ChaosManager::instance().get_state();
    return Rng::pdrift(result, drift);
  }
  return result;
}
