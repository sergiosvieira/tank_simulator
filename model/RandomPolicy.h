#ifndef RANDOMPOLICY_H
#define RANDOMPOLICY_H

#include "OffPolicy.h"

class RandomPolicy: public OffPolicy {
    DecisionResult decide(Task::PtrTask task, std::vector<RSU::PtrRSU>& rsus) override;
    double decision_time(Task::PtrTask task) override;
};


#endif // RANDOMPOLICY_H
