/**
 * @file run_scenario.cpp
 * @brief Execute manual/deterministic scenarios
 *
 * Usage:
 *   ./run_scenario <scenario_name> [--chaos] [seed]
 *
 * Available scenarios:
 *   - Oracle         : Optimal decisions for chaos
 *   - StressTest     : Extreme load test
 *   - Simple         : Basic validation
 *
 * Example:
 *   ./run_scenario Oracle --chaos 42
 */

#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "core/ChaosManager.h"
#include "core/Config.h"
#include "core/Simulator.h"
#include "events/SpecifiedTasksEvent.h"
#include "logger.h"
#include "metric.h"
#include "model/DeterministicPolicy.h"
#include "model/RSU.h"
#include "model/Vehicle.h"
#include "scenarios/OracleScenario.h"
#include "utils/Rng.h"
#include <iostream>
#include <memory>
#include <string>

using std::cout, std::endl;

void print_usage() {
  cout << "Usage: ./run_scenario <scenario_name> [--chaos] [seed]" << endl;
  cout << endl;
  cout << "Available scenarios:" << endl;
  cout << "  Oracle       - Optimal decisions for chaotic environment" << endl;
  cout << "  StressTest   - Extreme load stress test" << endl;
  cout << "  Simple       - Basic validation scenario" << endl;
  cout << endl;
  cout << "Options:" << endl;
  cout << "  --chaos      - Enable chaos mode (non-stationary)" << endl;
  cout << "  seed         - Random seed (default: 1978)" << endl;
}

std::unique_ptr<Scenario> create_scenario(const std::string &name) {
  if (name == "Oracle" || name == "oracle") {
    return std::make_unique<OracleScenario>();
  }
  if (name == "StressTest" || name == "stress") {
    return std::make_unique<StressTestScenario>();
  }
  if (name == "Simple" || name == "simple") {
    return std::make_unique<SimpleValidationScenario>();
  }
  return nullptr;
}

int main(int argc, char **argv) {
  if (argc < 2) {
    print_usage();
    return 1;
  }

  std::string scenario_name = argv[1];
  int seed = 1978;

  // Parse arguments
  for (int i = 2; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "--chaos") {
      Config::set_chaos_mode();
      cout << "!!! CHAOS MODE ACTIVATED !!!" << endl;
    } else if (arg[0] != '-') {
      seed = std::stoi(arg);
    }
  }

  // Create scenario
  auto scenario = create_scenario(scenario_name);
  if (!scenario) {
    cout << "Unknown scenario: " << scenario_name << endl;
    print_usage();
    return 1;
  }

  cout << "========================================" << endl;
  cout << "Running Scenario: " << scenario->name() << endl;
  cout << "Description: " << scenario->description() << endl;
  cout << "Seed: " << seed << endl;
  cout << "Expected Success Rate: " << (scenario->expected_success_rate() * 100)
       << "%" << endl;
  cout << "========================================" << endl;

  // Set seeds
  Rng::engine().seed(seed);
  ChaosManager::instance().seed(seed);

  // Create simulator
  Simulator sim;

  // Create scenario tasks and policy
  auto [tasks, policy] = scenario->create(sim);

  cout << "Tasks created: " << tasks.size() << endl;
  cout << "Decisions scripted: " << policy->script_size() << endl;

  // Setup metrics
  std::string result_file = "results/scenario_" + scenario->name() + "_" +
                            std::to_string(seed) + ".csv";
  MetricsHub::instance().clearListeners();
  auto csvCollector = std::make_shared<CSVMetricsCollector>(result_file);
  MetricsHub::instance().addListener(csvCollector);

  // Create RSUs
  std::vector<RSU::PtrRSU> rsus = {std::make_shared<RSU>()};
  for (auto r : rsus) {
    r->battery = Battery(10000.0);
    r->report_metric(sim, "BatteryRemaining", r->battery.get_remaining());
  }

  // Create vehicle with deterministic policy
  auto vehicle = std::make_shared<Vehicle>(policy);
  vehicle->set_rsus(rsus);
  vehicle->battery = Battery(10000.0);
  vehicle->report_metric(sim, "BatteryRemaining",
                         vehicle->battery.get_remaining());

  // Set tasks for SpecifiedTasksEvent
  SpecifiedTasksEvent::reset();
  SpecifiedTasksEvent::tasks = tasks;

  // Schedule first task
  if (!tasks.empty()) {
    sim.schedule<SpecifiedTasksEvent>(tasks[0]->get_timestamp(), vehicle);
  }

  // Calculate duration based on last task
  double duration = 20.0; // Default
  if (!tasks.empty()) {
    duration = tasks.back()->get_timestamp() + 10.0;
  }

  cout << "Simulation duration: " << duration << "s" << endl;
  cout << "Results file: " << result_file << endl;
  cout << "========================================" << endl;

  // Run simulation
  sim.run(duration);

  // Chaos validation
  if (Config::FIELD_TOTAL_CHAOS) {
    const auto &z = ChaosManager::instance().history();
    if (z.size() > 2) {
      double num = 0.0, den = 0.0;
      for (size_t t = 1; t < z.size(); ++t) {
        num += z[t] * z[t - 1];
        den += z[t - 1] * z[t - 1];
      }
      double rho_hat = (den > 0.0) ? num / den : 0.0;

      cout << "---------------------------------------" << endl;
      cout << "Chaos Temporal Validation" << endl;
      cout << "Estimated AR(1) rho_hat = " << rho_hat << endl;
      cout << "Samples collected = " << z.size() << endl;
      cout << "---------------------------------------" << endl;
    }
  }

  cout << "========================================" << endl;
  cout << "Scenario complete: " << scenario->name() << endl;
  cout << "========================================" << endl;

  return 0;
}
