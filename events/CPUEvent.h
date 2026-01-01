#ifndef CPUEVENT_H
#define CPUEVENT_H

#include "../core/Event.h"
#include "../model/Model.h"

class CPUEvent : public Event {
  Model::PtrModel model = nullptr;
  EventType type;

 public:
  CPUEvent(double t, Model::PtrModel model_, EventType type_)
      : Event(t), model(model_), type(type_) {}
  void execute(Simulator &sim) override { model->events[type](sim); }
};

#endif  // CPUEVENT_H
