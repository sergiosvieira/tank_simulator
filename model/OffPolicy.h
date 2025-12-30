#ifndef OFFPOLICY_H
#define OFFPOLICY_H

#include "Task.h"
#include "utils/Rng.h"
#include <memory>

class Model; // Forward declaration

enum class DecisionType { Local, Remote };

struct DecisionResult {
  DecisionType decision_type = DecisionType::Local;
  std::shared_ptr<Model> choosed_device = nullptr;
};

enum class OffPolicyState { Idle, Busy };

class OffPolicy {
  OffPolicyState current_state = OffPolicyState::Idle;

public:
  using PtrOffPolicy = std::shared_ptr<OffPolicy>;
  virtual DecisionResult decide(Task::PtrTask task);
  virtual double decision_time(Task::PtrTask task);
  void complete() { current_state = OffPolicyState::Idle; }
  void start() { current_state = OffPolicyState::Busy; }
  bool is_idle() const { return current_state == OffPolicyState::Idle; }
  bool is_busy() const { return current_state == OffPolicyState::Busy; }
};

#endif // OFFPOLICY_H
