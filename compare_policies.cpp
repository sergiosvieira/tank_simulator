/**
 * @file compare_policies.cpp
 * @brief Compare all policies on the same Oracle scenario tasks
 *
 * Usage:
 *   ./compare_policies [--chaos] [seed]
 */

#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

#include "core/ChaosManager.h"
#include "core/Config.h"
#include "core/Simulator.h"
#include "events/SpecifiedTasksEvent.h"
#include "logger.h"
#include "metric.h"
#include "model/DeterministicPolicy.h"
#include "model/FirstRemotePolicy.h"
#include "model/IntelligentPolicy.h"
#include "model/OffPolicy.h"
#include "model/RSU.h"
#include "model/RandomPolicy.h"
#include "model/Vehicle.h"
#include "scenarios/OracleScenario.h"
#include "utils/Rng.h"

using std::cout, std::endl;

struct PolicyResult {
  std::string name;
  int total_tasks = 0;
  int successful = 0;
  int failed = 0;
  double success_rate = 0.0;
  double avg_latency = 0.0;
  double total_energy = 0.0;
  int local_count = 0;
  int remote_count = 0;
};

PolicyResult run_with_policy(const std::string &policy_name,
                             std::shared_ptr<OffPolicy> policy, double duration,
                             int seed,
                             const std::vector<ScenarioTask> &scenario_tasks) {
  PolicyResult result;
  result.name = policy_name;

  // Reset seeds for reproducibility
  Rng::engine().seed(seed);
  ChaosManager::instance().seed(seed);
  ChaosManager::instance().reset();

  // Create fresh copies of tasks
  std::vector<Task::PtrTask> tasks;
  for (const auto &st : scenario_tasks) {
    auto task = std::make_shared<Task>(st.timestamp, st.size_bytes,
                                       st.density_cycles_bytes, st.deadline);
    tasks.push_back(task);
  }

  result.total_tasks = tasks.size();

  // Setup Oracle decisions if this is DeterministicPolicy
  auto det_policy = std::dynamic_pointer_cast<DeterministicPolicy>(policy);
  if (det_policy) {
    for (size_t i = 0; i < tasks.size() && i < scenario_tasks.size(); ++i) {
      det_policy->add_decision(tasks[i]->get_id(), scenario_tasks[i].decision);
    }
  }

  // Create simulator
  Simulator sim;

  // Setup metrics file
  std::string result_file =
      "results/compare_" + policy_name + "_" + std::to_string(seed) + ".csv";
  MetricsHub::instance().clearListeners();
  auto csvCollector = std::make_shared<CSVMetricsCollector>(result_file);
  MetricsHub::instance().addListener(csvCollector);

  // Create RSUs
  std::vector<RSU::PtrRSU> rsus = {std::make_shared<RSU>()};
  for (auto r : rsus) {
    r->battery = Battery(10000.0);
  }

  // Create vehicle with the policy
  auto vehicle = std::make_shared<Vehicle>(policy);
  vehicle->set_rsus(rsus);
  vehicle->battery = Battery(10000.0);

  // Set tasks for SpecifiedTasksEvent
  SpecifiedTasksEvent::reset();
  SpecifiedTasksEvent::tasks = tasks;

  // Schedule first task
  if (!tasks.empty()) {
    sim.schedule<SpecifiedTasksEvent>(tasks[0]->get_timestamp(), vehicle);
  }

  // Run simulation
  sim.run(duration);

  // Force flush metrics - must flush before reading!
  csvCollector->flush();
  MetricsHub::instance().clearListeners();

  // Parse results from CSV
  std::ifstream csv_file(result_file);
  std::string line;
  std::getline(csv_file, line);  // Skip header

  std::vector<double> latencies;
  double initial_battery = 10000.0;
  double final_battery = 10000.0;

  while (std::getline(csv_file, line)) {
    std::stringstream ss(line);
    std::string time_str, entity_str, metric, value_str, tag, taskid_str;
    std::getline(ss, time_str, ',');
    std::getline(ss, entity_str, ',');
    std::getline(ss, metric, ',');
    std::getline(ss, value_str, ',');
    std::getline(ss, tag, ',');
    std::getline(ss, taskid_str, ',');

    double value = 0;
    try {
      value = std::stod(value_str);
    } catch (...) {
    }

    if (metric == "TaskSuccess") {
      if (value >= 0.5)
        result.successful++;
      else
        result.failed++;
    } else if (metric == "TaskLatency") {
      latencies.push_back(value);
    } else if (metric == "OffloadingType") {
      if (value < 0.5)
        result.local_count++;
      else
        result.remote_count++;
    } else if (metric == "BatteryRemaining") {
      final_battery = value;
    }
  }
  csv_file.close();

  // Calculate stats
  if (result.total_tasks > 0) {
    result.success_rate =
        (double)result.successful / result.total_tasks * 100.0;
  }
  if (!latencies.empty()) {
    double sum = 0;
    for (double l : latencies) sum += l;
    result.avg_latency = sum / latencies.size();
  }
  result.total_energy = initial_battery - final_battery;

  return result;
}

int main(int argc, char **argv) {
  int seed = 1978;
  bool chaos_mode = false;

  // Parse arguments
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "--chaos") {
      chaos_mode = true;
      Config::set_chaos_mode();
    } else if (arg[0] != '-') {
      seed = std::stoi(arg);
    }
  }

  cout << "========================================" << endl;
  cout << "POLICY COMPARISON ON ORACLE SCENARIO" << endl;
  cout << "========================================" << endl;
  cout << "Seed: " << seed << endl;
  cout << "Chaos Mode: " << (chaos_mode ? "ON" : "OFF") << endl;
  cout << "========================================" << endl;

  // Get scenario tasks (same for all policies)
  NightmareScenario scenario;
  auto scenario_tasks = scenario.build_tasks();

  // Calculate duration
  double duration = 20.0;
  if (!scenario_tasks.empty()) {
    duration = scenario_tasks.back().timestamp + 10.0;
  }

  cout << "Tasks in scenario: " << scenario_tasks.size() << endl;
  cout << "Duration: " << duration << "s" << endl;
  cout << "========================================" << endl << endl;

  // Run each policy
  std::vector<PolicyResult> results;

  // Oracle (DeterministicPolicy)
  cout << "Running Oracle..." << endl;
  auto oracle_result =
      run_with_policy("Oracle", std::make_shared<DeterministicPolicy>(),
                      duration, seed, scenario_tasks);
  results.push_back(oracle_result);
  cout << "  Success: " << oracle_result.successful << "/"
       << oracle_result.total_tasks << " (" << oracle_result.success_rate
       << "%)" << endl;

  // Local
  cout << "Running Local..." << endl;
  auto local_result = run_with_policy("Local", std::make_shared<OffPolicy>(),
                                      duration, seed, scenario_tasks);
  results.push_back(local_result);
  cout << "  Success: " << local_result.successful << "/"
       << local_result.total_tasks << " (" << local_result.success_rate << "%)"
       << endl;

  // Random
  cout << "Running Random..." << endl;
  auto random_result =
      run_with_policy("Random", std::make_shared<RandomPolicy>(), duration,
                      seed, scenario_tasks);
  results.push_back(random_result);
  cout << "  Success: " << random_result.successful << "/"
       << random_result.total_tasks << " (" << random_result.success_rate
       << "%)" << endl;

  // FirstRemote
  cout << "Running FirstRemote..." << endl;
  auto first_result =
      run_with_policy("FirstRemote", std::make_shared<FirstRemotePolicy>(),
                      duration, seed, scenario_tasks);
  results.push_back(first_result);
  cout << "  Success: " << first_result.successful << "/"
       << first_result.total_tasks << " (" << first_result.success_rate << "%)"
       << endl;

  // Intelligent
  cout << "Running Intelligent..." << endl;
  auto intel_result =
      run_with_policy("Intelligent", std::make_shared<IntelligentPolicy>(),
                      duration, seed, scenario_tasks);
  results.push_back(intel_result);
  cout << "  Success: " << intel_result.successful << "/"
       << intel_result.total_tasks << " (" << intel_result.success_rate << "%)"
       << endl;

  // Print comparison table
  cout << endl;
  cout << "========================================" << endl;
  cout << "COMPARISON RESULTS" << endl;
  cout << "========================================" << endl;
  cout << std::fixed << std::setprecision(1);

  // Header
  cout << std::left << std::setw(15) << "Policy" << std::right << std::setw(10)
       << "Success" << std::setw(12) << "Rate(%)" << std::setw(12)
       << "Latency(s)" << std::setw(10) << "Local" << std::setw(10) << "Remote"
       << std::setw(12) << "Energy(J)" << endl;
  cout << std::string(81, '-') << endl;

  for (const auto &r : results) {
    cout << std::left << std::setw(15) << r.name << std::right << std::setw(5)
         << r.successful << "/" << std::setw(3) << r.total_tasks
         << std::setw(12) << r.success_rate << std::setw(12)
         << std::setprecision(4) << r.avg_latency << std::setprecision(1)
         << std::setw(10) << r.local_count << std::setw(10) << r.remote_count
         << std::setw(12) << std::setprecision(2) << r.total_energy << endl;
  }

  cout << "========================================" << endl;

  // Write summary CSV
  std::ofstream summary("results/policy_comparison_summary.csv");
  summary << "Policy,TotalTasks,Successful,Failed,SuccessRate,AvgLatency,"
             "LocalCount,RemoteCount,TotalEnergy\n";
  for (const auto &r : results) {
    summary << r.name << "," << r.total_tasks << "," << r.successful << ","
            << r.failed << "," << r.success_rate << "," << r.avg_latency << ","
            << r.local_count << "," << r.remote_count << "," << r.total_energy
            << "\n";
  }
  summary.close();
  cout << "Summary saved to: results/policy_comparison_summary.csv" << endl;

  return 0;
}
