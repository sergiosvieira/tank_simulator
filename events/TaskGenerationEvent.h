#ifndef TASKGENERATIONEVENT_H
#define TASKGENERATIONEVENT_H

#include "../core/Event.h"
#include "../model/Vehicle.h"

class TaskGenerationEvent : public Event {
  Vehicle::PtrVehicle model = nullptr;
  double lambda = 0.0;

public:
  TaskGenerationEvent(double t, Vehicle::PtrVehicle model_,
                      double lambda_ = 2.0)
      : Event(t), model(model_), lambda(lambda_) {}
  void execute(Simulator &sim) override;
};

#endif // TASKGENERATIONEVENT_H
