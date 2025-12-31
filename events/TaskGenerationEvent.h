#ifndef TASKGENERATIONEVENT_H
#define TASKGENERATIONEVENT_H

#include "../core/Event.h"
#include "../model/Vehicle.h"

static constexpr double Lambda = 1.0 / 0.02;

class TaskGenerationEvent : public Event {
  Vehicle::PtrVehicle model = nullptr;
  double lambda = Lambda;

public:
  TaskGenerationEvent(double t, Vehicle::PtrVehicle model_,
                      double lambda_ = Lambda)
      : Event(t), model(model_), lambda(lambda_) {}
  void execute(Simulator &sim) override;
};

#endif // TASKGENERATIONEVENT_H
