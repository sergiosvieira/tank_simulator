#include "OffPolicy.h"
#include "model/RSU.h"

DecisionResult OffPolicy::decide(Task::PtrTask task,
    std::vector<RSU::PtrRSU>& rsus) {
  return {DecisionType::Local, nullptr};
}

double OffPolicy::decision_time(Task::PtrTask task) {
  return Rng::uniform(0.003, 0.005); // 30–50 ms
}
