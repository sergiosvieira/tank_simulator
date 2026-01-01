#ifndef FIRSTREMOTEPOLICY_H
#define FIRSTREMOTEPOLICY_H

#include "OffPolicy.h"

class FirstRemotePolicy : public OffPolicy {
 public:
  FirstRemotePolicy() : OffPolicy() { name = "FirstRemotePolicy"; }
  DecisionResult decide(Task::PtrTask task,
                        std::vector<RSU::PtrRSU>& rsus) override;
  double decision_time(Task::PtrTask task) override;
};

#endif  // FIRSTREMOTEPOLICY_H
