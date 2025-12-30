#include "OffPolicy.h"

DecisionResult OffPolicy::decide(Task::PtrTask task) {
  return {DecisionType::Local, nullptr};
}

double OffPolicy::decision_time(Task::PtrTask task) {
  return Rng::uniform(0.003, 0.005); // 30–50 ms
}
