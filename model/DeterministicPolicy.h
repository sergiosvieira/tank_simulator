#ifndef DETERMINISTICPOLICY_H
#define DETERMINISTICPOLICY_H

#include <map>
#include <vector>

#include "OffPolicy.h"

/**
 * @brief Deterministic Policy - Scripted Decision Making
 *
 * This policy allows complete manual control over task decisions.
 * You specify exactly what happens to each task: local or remote,
 * and which RSU to use for offloading.
 *
 * Usage:
 * 1. Create a decision script (map of task_id -> decision)
 * 2. Set the script using set_decisions()
 * 3. The policy will follow the script exactly
 *
 * This is useful for:
 * - Testing specific scenarios
 * - Creating Oracle/optimal baselines
 * - Debugging and validation
 * - Reproducible benchmarks
 */

struct ScriptedDecision {
  DecisionType type = DecisionType::Local;
  int rsu_index = -1;  // -1 = local, 0+ = RSU index

  ScriptedDecision() = default;
  ScriptedDecision(DecisionType t, int idx = -1) : type(t), rsu_index(idx) {}

  // Convenience constructors
  static ScriptedDecision Local() {
    return ScriptedDecision(DecisionType::Local, -1);
  }
  static ScriptedDecision Remote(int rsu_idx = 0) {
    return ScriptedDecision(DecisionType::Remote, rsu_idx);
  }
};

class DeterministicPolicy : public OffPolicy {
 public:
  using DecisionScript =
      std::map<int, ScriptedDecision>;  // task_id -> decision

  DeterministicPolicy() { name = "DeterministicPolicy"; }

  /**
   * @brief Set the decision script
   * @param script Map from task_id to ScriptedDecision
   */
  void set_decisions(const DecisionScript &script) { decisions_ = script; }

  /**
   * @brief Add a decision for a specific task
   * @param task_id The task ID
   * @param decision The decision (Local or Remote with RSU index)
   */
  void add_decision(int task_id, ScriptedDecision decision) {
    decisions_[task_id] = decision;
  }

  /**
   * @brief Add multiple decisions at once
   * Format: {task_id, DecisionType, rsu_index}
   */
  void add_decisions(
      std::initializer_list<std::tuple<int, DecisionType, int>> list) {
    for (const auto &[id, type, rsu] : list) {
      decisions_[id] = ScriptedDecision(type, rsu);
    }
  }

  /**
   * @brief Set all tasks from a range to be processed locally
   */
  void set_all_local(int from_id, int to_id) {
    for (int i = from_id; i <= to_id; ++i) {
      decisions_[i] = ScriptedDecision::Local();
    }
  }

  /**
   * @brief Set all tasks from a range to be offloaded
   */
  void set_all_remote(int from_id, int to_id, int rsu_idx = 0) {
    for (int i = from_id; i <= to_id; ++i) {
      decisions_[i] = ScriptedDecision::Remote(rsu_idx);
    }
  }

  /**
   * @brief Make decision based on script
   * Falls back to Local if task_id not in script
   */
  DecisionResult decide(Task::PtrTask task,
                        std::vector<RSU::PtrRSU> &rsus) override {
    int task_id = task->get_id();

    // Lookup in script
    auto it = decisions_.find(task_id);
    if (it == decisions_.end()) {
      // Default: process locally if not in script
      return {DecisionType::Local, nullptr};
    }

    const auto &decision = it->second;

    if (decision.type == DecisionType::Local) {
      return {DecisionType::Local, nullptr};
    }

    // Remote: select RSU
    if (rsus.empty()) {
      // No RSUs available, fall back to local
      return {DecisionType::Local, nullptr};
    }

    int rsu_idx = decision.rsu_index;
    if (rsu_idx < 0 || rsu_idx >= static_cast<int>(rsus.size())) {
      rsu_idx = 0;  // Default to first RSU
    }

    return {DecisionType::Remote, rsus[rsu_idx]};
  }

  /**
   * @brief Decision time for deterministic policy
   * Minimal time since decision is pre-computed
   */
  double decision_time(Task::PtrTask task) override {
    return 0.001;  // 1ms - essentially instant lookup
  }

  /**
   * @brief Clear all decisions
   */
  void clear() { decisions_.clear(); }

  /**
   * @brief Get the number of scripted decisions
   */
  size_t script_size() const { return decisions_.size(); }

 private:
  DecisionScript decisions_;
};

#endif  // DETERMINISTICPOLICY_H
