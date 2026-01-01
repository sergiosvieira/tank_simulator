#ifndef SPECIFIEDTASKSEVENT_H
#define SPECIFIEDTASKSEVENT_H

#include "../core/Event.h"
#include "../model/Vehicle.h"

class SpecifiedTasksEvent : public Event {
  Vehicle::PtrVehicle model = nullptr;
  static size_t index;

 public:
  static std::vector<Task::PtrTask> tasks;
  static void reset() { index = 0; }

  SpecifiedTasksEvent(double t, Vehicle::PtrVehicle model_)
      : Event(t), model(model_) {}
  void execute(Simulator &sim) override;
};

#endif  // SPECIFIEDTASKSEVENT_H
