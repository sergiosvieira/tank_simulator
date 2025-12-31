#include "Task.h"

Task::Task(const Simulator &sim) {
  timestamp = sim.now();

  // --------------------------------------------------
  // Task Size (Regime Switching)
  // --------------------------------------------------
  if (Config::FIELD_TOTAL_CHAOS && Rng::uniform(0.0, 1.0) < 0.15) {
    // Reduced burst: 15% chance, max 600KB (was 30% chance, max 2MB)
    size_bytes = Rng::uniform(300000, 600000);
  } else {
    size_bytes = Rng::uniform(Config::TASK_MIN_SIZE, Config::TASK_MAX_SIZE);
  }

  // --------------------------------------------------
  // Density (High variance)
  // --------------------------------------------------
  if (Config::FIELD_TOTAL_CHAOS) {
    // Reduced variance: 2x instead of 3x
    density_cycles_bytes =
        Rng::normal(Config::TASK_MEAN_DENSITY, 2.0 * Config::TASK_STD_DENSITY);
  } else {
    density_cycles_bytes =
        Rng::normal(Config::TASK_MEAN_DENSITY, Config::TASK_STD_DENSITY);
  }

  // --------------------------------------------------
  // Deadline (Uniform distribution)
  // --------------------------------------------------
  deadline = Rng::uniform(Config::TASK_MIN_DEADLINE, Config::TASK_MAX_DEADLINE);
}

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
