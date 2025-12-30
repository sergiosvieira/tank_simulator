#ifndef CPU_H
#define CPU_H

#include "Task.h"

enum class CPUState { Idle, Busy };

class CPU {
  CPUState current_state = CPUState::Idle;
  double freq_mhz = 1.0 * 1e9;

public:
  virtual double processing_time(Task::PtrTask task);
  void complete() { current_state = CPUState::Idle; }
  void start() { current_state = CPUState::Busy; }
  bool is_idle() const { return current_state == CPUState::Idle; }
  bool is_busy() const { return current_state == CPUState::Busy; }
  double get_freq() const { return freq_mhz; }
  void set_freq(double freq_mhz_) { freq_mhz = freq_mhz_; }
};

#endif // CPU_H
