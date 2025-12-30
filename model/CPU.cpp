#include "CPU.h"

double CPU::processing_time(Task::PtrTask task) {
  return task->total_cycles() / freq_mhz;
}
