#ifndef INTELLIGENTPOLICY_H
#define INTELLIGENTPOLICY_H

#include <limits>

#include "OffPolicy.h"

class IntelligentPolicy : public OffPolicy {
 public:
  IntelligentPolicy() : OffPolicy() { name = "IntelligentPolicy"; }

  DecisionResult decide(Task::PtrTask task,
                        std::vector<RSU::PtrRSU>& rsus) override;
  double decision_time(Task::PtrTask task) override;
};

#endif  // INTELLIGENTPOLICY_H
