#include "Task.h"

Task::Task(const Simulator &sim) { timestamp = sim.now(); }

double Task::spent_time(Simulator &sim) { return sim.now() - timestamp; }

std::ostream &operator<<(std::ostream &out, const Task &t) {
  out << "Task { "
      << "timestamp=" << t.timestamp << ", id=" << t.id
      << ", size_bytes=" << t.size_bytes
      << ", density=" << t.density_cycles_bytes
      << ", total_cycles=" << t.total_cycles() << ", deadline=" << t.deadline
      << " }";
  return out;
}
