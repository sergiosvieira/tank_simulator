#pragma once

/**
 * @file Scenario.h
 * @brief Base class for manual test scenarios
 *
 * A Scenario defines:
 * 1. A set of pre-defined tasks (with specific sizes, deadlines, etc.)
 * 2. A decision script (which tasks go where)
 * 3. Expected outcomes for validation
 */

#include <functional>
#include <string>
#include <vector>

#include "../core/Simulator.h"
#include "../model/DeterministicPolicy.h"
#include "../model/Task.h"

struct ScenarioTask {
  double timestamp;
  long size_bytes;
  long density_cycles_bytes;
  double deadline;
  ScriptedDecision decision;

  ScenarioTask(double ts, long size, long density, double dl,
               ScriptedDecision dec)
      : timestamp(ts),
        size_bytes(size),
        density_cycles_bytes(density),
        deadline(dl),
        decision(dec) {}
};

class Scenario {
 public:
  virtual ~Scenario() = default;

  /**
   * @brief Get scenario name
   */
  virtual std::string name() const = 0;

  /**
   * @brief Get scenario description
   */
  virtual std::string description() const = 0;

  /**
   * @brief Build the tasks for this scenario
   * @return Vector of tasks with their decisions
   */
  virtual std::vector<ScenarioTask> build_tasks() const = 0;

  /**
   * @brief Get expected success rate (for validation)
   * @return Expected success rate [0, 1]
   */
  virtual double expected_success_rate() const { return 1.0; }

  /**
   * @brief Create Task objects and decision script
   * @param sim Simulator reference (for Task constructor)
   * @return Pair of (tasks, policy with decisions set)
   */
  std::pair<std::vector<Task::PtrTask>, std::shared_ptr<DeterministicPolicy>>
  create(Simulator &sim) const {
    auto scenario_tasks = build_tasks();
    std::vector<Task::PtrTask> tasks;
    auto policy = std::make_shared<DeterministicPolicy>();

    for (const auto &st : scenario_tasks) {
      auto task = std::make_shared<Task>(st.timestamp, st.size_bytes,
                                         st.density_cycles_bytes, st.deadline);
      tasks.push_back(task);
      policy->add_decision(task->get_id(), st.decision);
    }

    return {tasks, policy};
  }
};
