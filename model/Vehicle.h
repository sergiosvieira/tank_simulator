#ifndef VEHICLE_H
#define VEHICLE_H

#include <armadillo>
#include <queue>
#include <vector>

#include "Model.h"
#include "OffPolicy.h"
#include "model/RSU.h"

class Vehicle : public Model {
  arma::vec pos = {};
  arma::vec vel = {};
  std::vector<RSU::PtrRSU> rsus;
  std::queue<Task::PtrTask> decision_queue;
  Task::PtrTask decision_task = nullptr;
  OffPolicy::PtrOffPolicy off_policy = nullptr;

 public:
  using PtrVehicle = std::shared_ptr<Vehicle>;
  Vehicle(OffPolicy::PtrOffPolicy policy);
  void add_task_to_decision(Simulator &sim, Task::PtrTask task);
  void onDecisionStart(Simulator &sim);
  void onDecisionComplete(Simulator &sim);
  void set_rsus(const std::vector<RSU::PtrRSU> &rsus_);
  const std::vector<RSU::PtrRSU> get_rsus() const { return rsus; }

 protected:
  void schedule_decision(Simulator &sim);
  void schedule_decision_start_event(Simulator &sim);
};

#endif  // VEHICLE_H
