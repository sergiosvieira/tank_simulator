#ifndef VEHICLE_H
#define VEHICLE_H

#include "Model.h"
#include "OffPolicy.h"
#include <armadillo>
#include <queue>
#include <vector>

class Vehicle : public Model {
  arma::vec pos = {};
  arma::vec vel = {};
  std::vector<PtrModel> rsus;
  std::queue<Task::PtrTask> decision_queue;
  Task::PtrTask decision_task = nullptr;

public:
  using PtrVehicle = std::shared_ptr<Vehicle>;

  OffPolicy off_policy;

  Vehicle();

  void add_task_to_decision(Simulator &sim, Task::PtrTask task);

  void onDecisionStart(Simulator &sim);
  void onDecisionComplete(Simulator &sim);

protected:
  void schedule_decision(Simulator &sim);
  void schedule_decision_start_event(Simulator &sim);
};

#endif // VEHICLE_H
