#pragma once

/**
 * @file OracleScenario.h
 * @brief Oracle/Optimal scenario for chaos mode comparison
 */

#include "Scenario.h"

class OracleScenario : public Scenario {
public:
  std::string name() const override { return "OracleOptimal"; }

  std::string description() const override {
    return "Oracle with perfect knowledge - optimal decisions";
  }

  std::vector<ScenarioTask> build_tasks() const override {
    std::vector<ScenarioTask> tasks;

    // Phase 1: Normal load
    tasks.push_back({1.0, 50000, 800, 0.5, ScriptedDecision::Local()});
    tasks.push_back({1.2, 60000, 900, 0.4, ScriptedDecision::Local()});
    tasks.push_back({1.5, 55000, 850, 0.6, ScriptedDecision::Local()});
    tasks.push_back({2.0, 200000, 1000, 1.0, ScriptedDecision::Remote(0)});
    tasks.push_back({2.5, 70000, 750, 0.5, ScriptedDecision::Local()});
    tasks.push_back({3.0, 45000, 800, 0.4, ScriptedDecision::Local()});
    tasks.push_back({3.5, 350000, 1200, 1.5, ScriptedDecision::Remote(0)});
    tasks.push_back({4.0, 80000, 900, 0.6, ScriptedDecision::Local()});
    tasks.push_back({4.5, 65000, 850, 0.5, ScriptedDecision::Local()});

    // Phase 2: Burst
    tasks.push_back({5.0, 280000, 1100, 0.8, ScriptedDecision::Remote(0)});
    tasks.push_back({5.1, 300000, 1000, 0.9, ScriptedDecision::Remote(0)});
    tasks.push_back({5.2, 250000, 1200, 0.7, ScriptedDecision::Remote(0)});
    tasks.push_back({5.5, 40000, 600, 0.3, ScriptedDecision::Local()});
    tasks.push_back({6.0, 320000, 1100, 1.0, ScriptedDecision::Remote(0)});
    tasks.push_back({6.2, 290000, 950, 0.9, ScriptedDecision::Remote(0)});
    tasks.push_back({6.5, 35000, 700, 0.4, ScriptedDecision::Local()});
    tasks.push_back({7.0, 270000, 1050, 1.1, ScriptedDecision::Remote(0)});
    tasks.push_back({7.5, 310000, 1150, 1.2, ScriptedDecision::Remote(0)});

    // Phase 3: Recovery
    tasks.push_back({10.0, 150000, 900, 0.8, ScriptedDecision::Local()});
    tasks.push_back({11.0, 180000, 950, 1.0, ScriptedDecision::Remote(0)});
    tasks.push_back({12.0, 100000, 800, 0.6, ScriptedDecision::Local()});
    tasks.push_back({13.0, 220000, 1000, 1.2, ScriptedDecision::Remote(0)});
    tasks.push_back({14.0, 90000, 850, 0.5, ScriptedDecision::Local()});

    return tasks;
  }

  double expected_success_rate() const override { return 0.95; }
};

class StressTestScenario : public Scenario {
public:
  std::string name() const override { return "StressTest"; }
  std::string description() const override { return "Extreme stress test"; }

  std::vector<ScenarioTask> build_tasks() const override {
    std::vector<ScenarioTask> tasks;
    for (int i = 0; i < 20; ++i) {
      double ts = 1.0 + i * 0.1;
      long size = 200000 + (i % 5) * 50000;
      long density = 900 + (i % 3) * 100;
      double deadline = 0.5 + (i % 4) * 0.2;
      auto decision = (i % 2 == 0) ? ScriptedDecision::Remote(0)
                                   : ScriptedDecision::Local();
      tasks.push_back({ts, size, density, deadline, decision});
    }
    return tasks;
  }

  double expected_success_rate() const override { return 0.70; }
};

class SimpleValidationScenario : public Scenario {
public:
  std::string name() const override { return "SimpleValidation"; }
  std::string description() const override {
    return "Simple scenario for validation";
  }

  std::vector<ScenarioTask> build_tasks() const override {
    return {
        {1.0, 50000, 800, 1.0, ScriptedDecision::Local()},
        {2.0, 150000, 1000, 1.5, ScriptedDecision::Remote(0)},
        {3.0, 60000, 900, 1.0, ScriptedDecision::Local()},
    };
  }

  double expected_success_rate() const override { return 1.0; }
};

class HardcoreScenario : public Scenario {
public:
  std::string name() const override { return "Hardcore"; }
  std::string description() const override {
    return "EXTREME scenario designed to break simple policies";
  }

  std::vector<ScenarioTask> build_tasks() const override {
    std::vector<ScenarioTask> tasks;

    // Wave 1: Setup
    tasks.push_back({1.0, 50000, 800, 0.3, ScriptedDecision::Local()});
    tasks.push_back({1.1, 60000, 900, 0.35, ScriptedDecision::Local()});

    // Wave 2: First stress burst - alternating heavy/light
    tasks.push_back({2.0, 400000, 1200, 0.35, ScriptedDecision::Remote(0)});
    tasks.push_back({2.05, 20000, 500, 0.15, ScriptedDecision::Local()});
    tasks.push_back({2.1, 380000, 1100, 0.38, ScriptedDecision::Remote(0)});
    tasks.push_back({2.15, 25000, 600, 0.18, ScriptedDecision::Local()});
    tasks.push_back({2.2, 350000, 1000, 0.4, ScriptedDecision::Remote(0)});

    // Wave 3: Medium tasks - smart splitting
    tasks.push_back({3.0, 250000, 900, 0.45, ScriptedDecision::Remote(0)});
    tasks.push_back({3.1, 180000, 850, 0.42, ScriptedDecision::Local()});
    tasks.push_back({3.2, 280000, 950, 0.48, ScriptedDecision::Remote(0)});
    tasks.push_back({3.3, 150000, 800, 0.4, ScriptedDecision::Local()});
    tasks.push_back({3.4, 300000, 1000, 0.5, ScriptedDecision::Remote(0)});

    // Wave 4: Critical path
    tasks.push_back({4.0, 15000, 400, 0.1, ScriptedDecision::Local()});
    tasks.push_back({4.1, 500000, 1200, 0.7, ScriptedDecision::Remote(0)});
    tasks.push_back({4.2, 18000, 450, 0.12, ScriptedDecision::Local()});
    tasks.push_back({4.3, 450000, 1100, 0.65, ScriptedDecision::Remote(0)});
    tasks.push_back({4.4, 22000, 500, 0.14, ScriptedDecision::Local()});

    // Wave 5: Recovery
    tasks.push_back({5.0, 80000, 800, 0.5, ScriptedDecision::Local()});
    tasks.push_back({5.2, 150000, 1500, 0.4, ScriptedDecision::Remote(0)});
    tasks.push_back({5.4, 70000, 750, 0.45, ScriptedDecision::Local()});
    tasks.push_back({5.6, 120000, 1400, 0.38, ScriptedDecision::Remote(0)});
    tasks.push_back({5.8, 90000, 850, 0.5, ScriptedDecision::Local()});

    // Wave 6: Final burst - balanced
    tasks.push_back({7.0, 350000, 1100, 0.5, ScriptedDecision::Remote(0)});
    tasks.push_back({7.1, 30000, 600, 0.2, ScriptedDecision::Local()});
    tasks.push_back({7.2, 400000, 1200, 0.55, ScriptedDecision::Remote(0)});
    tasks.push_back({7.3, 25000, 550, 0.18, ScriptedDecision::Local()});

    return tasks;
  }

  double expected_success_rate() const override { return 0.90; }
};

/**
 * @brief NIGHTMARE Scenario - OPTIMIZED Oracle decisions
 *
 * This scenario breaks simple policies while Oracle achieves ~100%
 * Key: Deadlines are realistic and decisions are TRULY optimal
 */
class NightmareScenario : public Scenario {
public:
  std::string name() const override { return "Nightmare"; }

  std::string description() const override {
    return "EXTREME adversarial scenario - designed to destroy all heuristics";
  }

  std::vector<ScenarioTask> build_tasks() const override {
    std::vector<ScenarioTask> tasks;

    // ============================================
    // TRAP 1: RSU Congestion (t=1.0-1.3s)
    // FirstRemote will flood RSU, then critical items timeout
    // ============================================

    // Bait - heavy tasks for remote
    tasks.push_back({1.0, 500000, 1500, 0.8, ScriptedDecision::Remote(0)});
    tasks.push_back({1.02, 480000, 1400, 0.78, ScriptedDecision::Remote(0)});
    tasks.push_back({1.04, 520000, 1600, 0.82, ScriptedDecision::Remote(0)});

    // TRAP: Big data that CANNOT be offloaded (transfer > deadline)
    // FirstRemote will fail these!
    tasks.push_back(
        {1.1, 1500000, 30, 0.2,
         ScriptedDecision::Local()}); // 1.5MB, 200ms deadline, tx=357ms!
    tasks.push_back(
        {1.15, 1300000, 35, 0.18,
         ScriptedDecision::Local()}); // 1.3MB, 180ms deadline, tx=310ms!
    tasks.push_back(
        {1.2, 1100000, 40, 0.16,
         ScriptedDecision::Local()}); // 1.1MB, 160ms deadline, tx=262ms!

    // ============================================
    // TRAP 2: Deceptive Density (t=2.0-2.4s)
    // Small size but HUGE cycles - Random will fail
    // ============================================

    // These LOOK small but are 300M+ cycles! MUST be remote!
    tasks.push_back(
        {2.0, 80000, 4000, 0.25, ScriptedDecision::Remote(0)}); // 320M cycles!
    tasks.push_back(
        {2.05, 70000, 4500, 0.22, ScriptedDecision::Remote(0)}); // 315M cycles!
    tasks.push_back(
        {2.1, 90000, 3800, 0.28, ScriptedDecision::Remote(0)}); // 342M cycles!
    tasks.push_back(
        {2.15, 75000, 4200, 0.24, ScriptedDecision::Remote(0)}); // 315M cycles!

    // Actually small - local
    tasks.push_back({2.2, 30000, 500, 0.25, ScriptedDecision::Local()});
    tasks.push_back({2.25, 35000, 550, 0.28, ScriptedDecision::Local()});

    // More big data traps
    tasks.push_back({2.3, 2000000, 25, 0.25,
                     ScriptedDecision::Local()}); // 2MB! tx=476ms > deadline!
    tasks.push_back({2.4, 1800000, 28, 0.22,
                     ScriptedDecision::Local()}); // 1.8MB! tx=429ms > deadline!

    // ============================================
    // TRAP 3: Burst Overload (t=3.0-3.25s)
    // 10 tasks in 250ms - queue explodes
    // ============================================

    tasks.push_back({3.0, 400000, 1200, 0.5, ScriptedDecision::Remote(0)});
    tasks.push_back({3.02, 25000, 400, 0.12, ScriptedDecision::Local()});
    tasks.push_back({3.04, 380000, 1150, 0.48, ScriptedDecision::Remote(0)});
    tasks.push_back({3.06, 22000, 380, 0.11, ScriptedDecision::Local()});
    tasks.push_back({3.08, 420000, 1250, 0.52, ScriptedDecision::Remote(0)});
    tasks.push_back({3.1, 28000, 420, 0.13, ScriptedDecision::Local()});
    tasks.push_back({3.12, 390000, 1180, 0.49, ScriptedDecision::Remote(0)});
    tasks.push_back({3.14, 20000, 360, 0.1, ScriptedDecision::Local()});
    tasks.push_back({3.16, 410000, 1220, 0.51, ScriptedDecision::Remote(0)});
    tasks.push_back({3.18, 24000, 400, 0.115, ScriptedDecision::Local()});

    // ============================================
    // TRAP 4: Queue Depth Trap (t=4.0-4.3s)
    // Intelligent looks at queue and makes wrong choice
    // ============================================

    // Fill local queue
    tasks.push_back({4.0, 60000, 700, 0.4, ScriptedDecision::Local()});
    tasks.push_back({4.02, 65000, 750, 0.42, ScriptedDecision::Local()});
    tasks.push_back({4.04, 70000, 800, 0.44, ScriptedDecision::Local()});

    // Now Intelligent will offload - but these NEED local!
    // Big data - transfer time exceeds deadline
    tasks.push_back({4.06, 900000, 45, 0.14,
                     ScriptedDecision::Local()}); // tx=214ms > 140ms!
    tasks.push_back({4.08, 850000, 50, 0.13,
                     ScriptedDecision::Local()}); // tx=203ms > 130ms!
    tasks.push_back({4.1, 950000, 42, 0.15,
                     ScriptedDecision::Local()}); // tx=226ms > 150ms!

    // ============================================
    // TRAP 5: Pattern Breaking (t=5.0-5.6s)
    // L-R-L-R pattern then BREAK
    // ============================================

    tasks.push_back({5.0, 50000, 600, 0.35, ScriptedDecision::Local()});
    tasks.push_back({5.05, 300000, 1100, 0.5, ScriptedDecision::Remote(0)});
    tasks.push_back({5.1, 55000, 650, 0.38, ScriptedDecision::Local()});
    tasks.push_back({5.15, 320000, 1150, 0.52, ScriptedDecision::Remote(0)});
    // BREAK - three consecutive remote!
    tasks.push_back({5.2, 280000, 1050, 0.48, ScriptedDecision::Remote(0)});
    tasks.push_back({5.25, 290000, 1080, 0.5, ScriptedDecision::Remote(0)});
    tasks.push_back({5.3, 310000, 1120, 0.51, ScriptedDecision::Remote(0)});
    // Now local
    tasks.push_back({5.35, 45000, 580, 0.32, ScriptedDecision::Local()});

    // More big data traps!
    tasks.push_back({5.4, 1600000, 32, 0.21,
                     ScriptedDecision::Local()}); // 1.6MB! tx=381ms!
    tasks.push_back({5.5, 1400000, 36, 0.19,
                     ScriptedDecision::Local()}); // 1.4MB! tx=333ms!

    // ============================================
    // TRAP 6: Final Gauntlet (t=7.0-7.4s)
    // Maximum chaos - alternating with big data
    // ============================================

    tasks.push_back({7.0, 350000, 1200, 0.5, ScriptedDecision::Remote(0)});
    tasks.push_back({7.02, 1200000, 40, 0.17,
                     ScriptedDecision::Local()}); // tx=286ms > 170ms!
    tasks.push_back({7.04, 380000, 1250, 0.52, ScriptedDecision::Remote(0)});
    tasks.push_back({7.06, 1100000, 42, 0.16,
                     ScriptedDecision::Local()}); // tx=262ms > 160ms!
    tasks.push_back({7.08, 360000, 1180, 0.5, ScriptedDecision::Remote(0)});
    tasks.push_back({7.1, 1000000, 45, 0.15,
                     ScriptedDecision::Local()}); // tx=238ms > 150ms!
    tasks.push_back({7.12, 370000, 1220, 0.51, ScriptedDecision::Remote(0)});
    tasks.push_back({7.14, 900000, 48, 0.14,
                     ScriptedDecision::Local()}); // tx=214ms > 140ms!

    return tasks;
  }

  double expected_success_rate() const override { return 0.90; }
};
