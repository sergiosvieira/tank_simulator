#ifndef DECISIONEVENT_H
#define DECISIONEVENT_H

#include "../core/Event.h"
#include "../model/Vehicle.h"

class DecisionEvent : public Event {
  Vehicle::PtrVehicle model = nullptr;
  EventType type;

 public:
  DecisionEvent(double t, Vehicle::PtrVehicle model_, EventType type_)
      : Event(t), model(model_), type(type_) {}
  void execute(Simulator &sim) override { model->events[type](sim); }
};

#endif  // DECISIONEVENT_H
