#include "FirstRemotePolicy.h"
#include "Model.h"

DecisionResult FirstRemotePolicy::decide(Task::PtrTask task,
                                    std::vector<RSU::PtrRSU>& rsus) {
    DecisionType dt = DecisionType::Local;
    RSU::PtrRSU dst = nullptr;
    if (rsus.size() == 0) return {dt, dst};
    dt = DecisionType::Remote;
    dst = rsus[0];
    return {dt, dst};
}

double FirstRemotePolicy::decision_time(Task::PtrTask task) {
    return Rng::uniform(0.003, 0.005); // 30–50ms
}
