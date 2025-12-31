#pragma once

/**
 * @file OracleScenario.h
 * @brief Oracle/Optimal scenario for chaos mode comparison
 *
 * This scenario represents the BEST POSSIBLE decisions for a chaotic
 * environment. The Oracle has perfect knowledge of:
 * - Task sizes and deadlines
 * - RSU availability and capacity
 * - Network conditions
 *
 * Use this as an upper bound to compare against other policies.
 *
 * Strategy:
 * - Small tasks with short deadlines → Local (minimize latency)
 * - Large tasks with comfortable deadlines → Remote (offload heavy work)
 * - Heavy tasks with tight deadlines → Remote to fastest RSU
 * - During high chaos → More offloading to balance load
 */

#include "Scenario.h"

class OracleScenario : public Scenario {
public:
  std::string name() const override { return "OracleOptimal"; }

  std::string description() const override {
    return "Oracle with perfect knowledge - optimal decisions for chaotic "
           "environment";
  }

  /**
   * @brief Build optimal tasks for a challenging chaos scenario
   *
   * This creates a mix of tasks that would challenge simple policies
   * but can be handled optimally with the right decisions.
   */
  std::vector<ScenarioTask> build_tasks() const override {
    std::vector<ScenarioTask> tasks;

    // ============================================
    // Phase 1: Normal load (t=1-5s)
    // Mix of local and remote to establish baseline
    // ============================================

    // Small, fast tasks - process locally (minimal overhead)
    tasks.push_back({1.0, 50000, 800, 0.5, ScriptedDecision::Local()});
    tasks.push_back({1.2, 60000, 900, 0.4, ScriptedDecision::Local()});
    tasks.push_back({1.5, 55000, 850, 0.6, ScriptedDecision::Local()});

    // Medium task with comfortable deadline - offload
    tasks.push_back({2.0, 200000, 1000, 1.0, ScriptedDecision::Remote(0)});

    // Small tasks - keep local
    tasks.push_back({2.5, 70000, 750, 0.5, ScriptedDecision::Local()});
    tasks.push_back({3.0, 45000, 800, 0.4, ScriptedDecision::Local()});

    // Large task - definitely offload
    tasks.push_back({3.5, 350000, 1200, 1.5, ScriptedDecision::Remote(0)});

    tasks.push_back({4.0, 80000, 900, 0.6, ScriptedDecision::Local()});
    tasks.push_back({4.5, 65000, 850, 0.5, ScriptedDecision::Local()});

    // ============================================
    // Phase 2: Burst/Chaos period (t=5-10s)
    // Heavy load - Oracle offloads more aggressively
    // ============================================

    // Burst of heavy tasks - offload all to RSU
    tasks.push_back({5.0, 280000, 1100, 0.8, ScriptedDecision::Remote(0)});
    tasks.push_back({5.1, 300000, 1000, 0.9, ScriptedDecision::Remote(0)});
    tasks.push_back({5.2, 250000, 1200, 0.7, ScriptedDecision::Remote(0)});

    // Critical task - tight deadline, must be local for speed
    tasks.push_back({5.5, 40000, 600, 0.3, ScriptedDecision::Local()});

    // More heavy tasks during chaos
    tasks.push_back({6.0, 320000, 1100, 1.0, ScriptedDecision::Remote(0)});
    tasks.push_back({6.2, 290000, 950, 0.9, ScriptedDecision::Remote(0)});

    // Quick local task between heavy ones
    tasks.push_back({6.5, 35000, 700, 0.4, ScriptedDecision::Local()});

    tasks.push_back({7.0, 270000, 1050, 1.1, ScriptedDecision::Remote(0)});
    tasks.push_back({7.5, 310000, 1150, 1.2, ScriptedDecision::Remote(0)});

    // ============================================
    // Phase 3: Recovery (t=10-15s)
    // Load decreases, mix of decisions
    // ============================================

    tasks.push_back({10.0, 150000, 900, 0.8, ScriptedDecision::Local()});
    tasks.push_back({11.0, 180000, 950, 1.0, ScriptedDecision::Remote(0)});
    tasks.push_back({12.0, 100000, 800, 0.6, ScriptedDecision::Local()});
    tasks.push_back({13.0, 220000, 1000, 1.2, ScriptedDecision::Remote(0)});
    tasks.push_back({14.0, 90000, 850, 0.5, ScriptedDecision::Local()});

    return tasks;
  }

  double expected_success_rate() const override {
    return 0.95; // Oracle should achieve ~95%+ success
  }
};

/**
 * @brief Stress test scenario - maximum chaos
 */
class StressTestScenario : public Scenario {
public:
  std::string name() const override { return "StressTest"; }

  std::string description() const override {
    return "Extreme stress test - high load with tight deadlines";
  }

  std::vector<ScenarioTask> build_tasks() const override {
    std::vector<ScenarioTask> tasks;

    // Create burst of 20 heavy tasks in 2 seconds
    for (int i = 0; i < 20; ++i) {
      double ts = 1.0 + i * 0.1;
      long size = 200000 + (i % 5) * 50000;  // 200-400KB
      long density = 900 + (i % 3) * 100;    // 900-1100
      double deadline = 0.5 + (i % 4) * 0.2; // 0.5-1.1s

      // Alternate local/remote for load balancing
      auto decision = (i % 2 == 0) ? ScriptedDecision::Remote(0)
                                   : ScriptedDecision::Local();

      tasks.push_back({ts, size, density, deadline, decision});
    }

    return tasks;
  }

  double expected_success_rate() const override {
    return 0.70; // Even Oracle may struggle with extreme load
  }
};

/**
 * @brief Simple validation scenario
 */
class SimpleValidationScenario : public Scenario {
public:
  std::string name() const override { return "SimpleValidation"; }

  std::string description() const override {
    return "Simple scenario for validating basic functionality";
  }

  std::vector<ScenarioTask> build_tasks() const override {
    return {
        // Task 1: Small, local
        {1.0, 50000, 800, 1.0, ScriptedDecision::Local()},
        // Task 2: Medium, remote
        {2.0, 150000, 1000, 1.5, ScriptedDecision::Remote(0)},
        // Task 3: Small, local
        {3.0, 60000, 900, 1.0, ScriptedDecision::Local()},
    };
  }

  double expected_success_rate() const override {
    return 1.0; // Should be 100% success
  }
};
