#include "Task.h"
#include "../core/ChaosManager.h"

Task::Task(const Simulator &sim) {
  timestamp = sim.now();

  // --------------------------------------------------
  // Update Global Chaos State ONCE per task
  // This ensures all parameters share the SAME chaos value
  // --------------------------------------------------
  if (Config::FIELD_TOTAL_CHAOS) {
    ChaosManager::instance().update();
  }

  // Get the shared chaos state for this task
  double drift =
      Config::FIELD_TOTAL_CHAOS ? ChaosManager::instance().get_state() : 0.0;

  // --------------------------------------------------
  // Task Size (Drifted in chaos mode)
  // --------------------------------------------------
  if (Config::FIELD_TOTAL_CHAOS) {
    double min_s = Rng::pdrift(Config::TASK_MIN_SIZE, drift);
    double max_s = Rng::pdrift(Config::TASK_MAX_SIZE, drift);
    if (min_s > max_s) {
      std::swap(min_s, max_s);
    }
    size_bytes = Rng::uniform(min_s, max_s);
  } else {
    size_bytes = Rng::uniform(Config::TASK_MIN_SIZE, Config::TASK_MAX_SIZE);
  }

  // --------------------------------------------------
  // Density (Drifted in chaos mode)
  // --------------------------------------------------
  if (Config::FIELD_TOTAL_CHAOS) {
    double mean = Rng::pdrift(Config::TASK_MEAN_DENSITY, drift);
    double std = Rng::pdrift(Config::TASK_STD_DENSITY, drift);
    density_cycles_bytes = Rng::normal(mean, std);
  } else {
    density_cycles_bytes =
        Rng::normal(Config::TASK_MEAN_DENSITY, Config::TASK_STD_DENSITY);
  }

  // --------------------------------------------------
  // Deadline (Drifted in chaos mode)
  // --------------------------------------------------
  if (Config::FIELD_TOTAL_CHAOS) {
    double min_d = Rng::pdrift(Config::TASK_MIN_DEADLINE, drift);
    double max_d = Rng::pdrift(Config::TASK_MAX_DEADLINE, drift);
    if (min_d > max_d) {
      std::swap(min_d, max_d);
    }
    deadline = Rng::uniform(min_d, max_d);
  } else {
    deadline =
        Rng::uniform(Config::TASK_MIN_DEADLINE, Config::TASK_MAX_DEADLINE);
  }
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
