#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <exception>
#include <iostream>
#include <memory> /* for shared_ptr, make_shared */
#include <queue>
#include <stdexcept>
#include <vector>

#include "Event.h"

const double micro_step = 0.0001;

class Simulator {
  std::priority_queue<Event::PtrEvent, Event::VecPtrEvent, EventPtrComparator>
      fel;
  double current_time = 0.0;
  double end_time = 0.0;

 public:
  double now() const { return current_time; }

  void schedule(Event::PtrEvent event);

  template <typename T, typename... Args>
  void schedule(double t, Args &&...args) {
    schedule(std::make_shared<T>(t, std::forward<Args>(args)...));
  }

  void run(double sim_end_time);
  double get_end_time() const { return end_time; }
};

#endif  // SIMULATOR_H
