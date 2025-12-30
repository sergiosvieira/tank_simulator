#ifndef TASK_H
#define TASK_H

#include "core/Simulator.h" // Needed for constructor argument and method
#include "utils/IdManager.h"
#include "utils/Rng.h"
#include <iostream>
#include <memory>

class Task {
  double timestamp = 0;
  int id = IdManager::next_id();
  long size_bytes = Rng::uniform(90, 300);
  long density_cycles_bytes = Rng::normal(200000, 10000);
  double deadline = Rng::uniform(0.08, 0.1); // 80ms - 100ms
  bool offloaded = false;
  friend std::ostream &operator<<(std::ostream &out, const Task &t);

public:
  using PtrTask = std::shared_ptr<Task>;

  Task(const Simulator &sim);

  long total_cycles() const { return size_bytes * density_cycles_bytes; }
  int get_id() const { return id; }
  double get_deadline() const { return deadline; }
  double spent_time(Simulator &sim);
  bool get_offloaded() const { return offloaded; }
  void set_offloaded(bool val) { offloaded = val; }
};

std::ostream &operator<<(std::ostream &out, const Task &t);

#endif // TASK_H
