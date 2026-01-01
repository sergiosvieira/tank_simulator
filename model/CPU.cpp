#include "CPU.h"

#include "../core/ChaosManager.h"

double CPU::processing_time(Task::PtrTask task) {
  double result = task->total_cycles() / freq_mhz;
  if (Config::FIELD_TOTAL_CHAOS) {
    double drift = ChaosManager::instance().get_state();
    return Rng::pdrift(result, drift);
  }
  return result;
}
