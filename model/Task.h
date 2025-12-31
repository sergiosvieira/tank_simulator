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
  int origin_node_id = -1;                            // Added origin node ID
  long size_bytes = Rng::uniform(100000, 300000);     // 100KB - 300KB
  long density_cycles_bytes = Rng::normal(1000, 100); // 1000 cycles/byte
  double deadline = Rng::uniform(0.4, 0.5);           // 100ms - 200ms
  bool offloaded = false;
  friend std::ostream &operator<<(std::ostream &out, const Task &t);

public:
  using PtrTask = std::shared_ptr<Task>;

  Task(const Simulator &sim);
  Task(double timestamp_, long size_bytes_, long density_cycles_bytes_,
       double deadline_)
      : timestamp(timestamp_), size_bytes(size_bytes_),
        density_cycles_bytes(density_cycles_bytes_), deadline(deadline_){};
  long total_cycles() const { return size_bytes * density_cycles_bytes; }
  int get_id() const { return id; }
  void set_origin_node_id(int id) { origin_node_id = id; }
  int get_origin_node_id() const { return origin_node_id; }
  double get_deadline() const { return deadline; }
  double spent_time(Simulator &sim);
  bool get_offloaded() const { return offloaded; }
  void set_offloaded(bool val) { offloaded = val; }
  long get_data_size() const { return size_bytes; }
  long get_cycles() const { return density_cycles_bytes; }
  double get_timestamp() const { return timestamp; }
};

std::ostream &operator<<(std::ostream &out, const Task &t);

#endif // TASK_H
