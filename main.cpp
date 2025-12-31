#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifndef M_E
#define M_E 2.71828182845904523536
#endif

#include "core/Config.h" // Added Config
#include "core/Simulator.h"
#include "events/SpecifiedTasksEvent.h"
#include "events/TaskGenerationEvent.h"
#include "logger.h"
#include "metric.h"
#include "model/FirstRemotePolicy.h"
#include "model/IntelligentPolicy.h"
#include "model/OffPolicy.h"
#include "model/RSU.h"
#include "model/RandomPolicy.h"
#include "model/Task.h"
#include "model/Vehicle.h"
#include "utils/Rng.h"
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

using std::cout, std::endl;

std::shared_ptr<OffPolicy> create_policy(const std::string &name) {
  if (name == "Random")
    return std::make_shared<RandomPolicy>();
  if (name == "Intelligent")
    return std::make_shared<IntelligentPolicy>();
  if (name == "FirstRemote")
    return std::make_shared<FirstRemotePolicy>();
  return std::make_shared<OffPolicy>(); // Default: Local
}

std::string get_result_filename(const std::string &name, int seed) {
  std::map<std::string, std::string> suffix = {{"Local", "local"},
                                               {"Random", "random"},
                                               {"Intelligent", "intelligent"},
                                               {"FirstRemote", "first_remote"}};

  std::string s = "local";
  if (suffix.count(name))
    s = suffix[name];
  return "results/experiment_001_" + s + "_" + std::to_string(seed) + ".csv";
}

void calculate_scenario_entropy() {
  double lambda = Config::TRAFFIC_LAMBDA;

  // ---------- Proper relative uncertainty (non-negative, interpretable)
  // ----------

  // Exponential arrivals: mean inter-arrival time
  double U_arrival = (1.0 / lambda) / Config::REF_ARRIVAL_MAX_MEAN;

  // Uniform size span
  double U_size =
      (Config::TASK_MAX_SIZE - Config::TASK_MIN_SIZE) / Config::REF_SIZE_SPAN;

  // Normal density dispersion
  double U_density = Config::TASK_STD_DENSITY / Config::REF_DENSITY_STD_MAX;

  // Uniform deadline span
  double U_deadline = (Config::TASK_MAX_DEADLINE - Config::TASK_MIN_DEADLINE) /
                      Config::REF_DEADLINE_SPAN;

  // Optional logarithmic compression (safe)
  auto compress = [](double u) { return std::log(1.0 + u); };

  double UN_arrival = compress(U_arrival);
  double UN_size = compress(U_size);
  double UN_density = compress(U_density);
  double UN_deadline = compress(U_deadline);

  // ---------- Aggregated Scenario Uncertainty ----------
  double scenario_uncertainty_index =
      UN_arrival + UN_size + UN_density + UN_deadline;

  cout << ">>> Scenario Uncertainty Analysis (Proper Relative) <<<" << endl;
  cout << "Arrivals  | U = " << UN_arrival << endl;
  cout << "Task Size | U = " << UN_size << endl;
  cout << "Density   | U = " << UN_density << endl;
  cout << "Deadline  | U = " << UN_deadline << endl;

  cout << "---------------------------------------" << endl;
  cout << "Scenario Uncertainty Index: " << scenario_uncertainty_index << endl;
  cout << "---------------------------------------" << endl;
}

int main(int argc, char **argv) {
  std::string policy_name = "Local";
  double duration = 100.0;
  int seed = 1978;

  if (argc > 1)
    policy_name = argv[1];
  if (argc > 2)
    duration = std::stod(argv[2]);
  if (argc > 3)
    seed = std::stoi(argv[3]);

  // Set Global Seed
  Rng::engine().seed(seed);

  cout << "Running experiment with Policy: " << policy_name
       << " | Duration: " << duration << " | Seed: " << seed << endl;

  calculate_scenario_entropy();

  Simulator sim;
  auto policy = create_policy(policy_name);
  std::string result_file = get_result_filename(policy_name, seed);
  cout << "Results will be saved to: " << result_file << endl;

  MetricsHub::instance().clearListeners();
  auto csvCollector = std::make_shared<CSVMetricsCollector>(result_file);
  MetricsHub::instance().addListener(csvCollector);

  std::vector<Vehicle::PtrVehicle> vehicles = {
      std::make_shared<Vehicle>(policy),
      std::make_shared<Vehicle>(policy),
      std::make_shared<Vehicle>(policy),
  };
  std::vector<RSU::PtrRSU> rsus = {std::make_shared<RSU>()};
  for (auto r : rsus) {
    r->battery = Battery(10000.0); // Set 10kJ battery
    r->report_metric(sim, "BatteryRemaining", r->battery.get_remaining());
  }

  for (auto v : vehicles) {
    v->set_rsus(rsus);
    v->battery = Battery(10000.0); // Set 10kJ battery
    v->report_metric(sim, "BatteryRemaining", v->battery.get_remaining());
    sim.schedule<TaskGenerationEvent>(1.0, v, Config::TRAFFIC_LAMBDA);
  }

  sim.run(duration);
  return 0;
}
