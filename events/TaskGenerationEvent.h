#ifndef TASKGENERATIONEVENT_H
#define TASKGENERATIONEVENT_H

#include "../core/Config.h" // Added Config
#include "../core/Event.h"
#include "../model/Vehicle.h"

class TaskGenerationEvent : public Event {
  Vehicle::PtrVehicle model = nullptr;
  double lambda = Config::TRAFFIC_LAMBDA;

public:
  TaskGenerationEvent(double t, Vehicle::PtrVehicle model_,
                      double lambda_ = Config::TRAFFIC_LAMBDA)
      : Event(t), model(model_), lambda(lambda_) {}
  void execute(Simulator &sim) override;
};

#endif // TASKGENERATIONEVENT_H
