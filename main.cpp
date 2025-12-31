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

  auto policy = create_policy(policy_name);
  std::string result_file = get_result_filename(policy_name, seed);
  cout << "Results will be saved to: " << result_file << endl;

  // Setup Tasks (Rng used inside Task constructor will valid seed)
  // std::vector<Task::PtrTask> tasks = {
  //     std::make_shared<Task>(1.0, 300, 300000, 0.08),
  //     std::make_shared<Task>(2.0, 200, 200000, 0.04)};
  // SpecifiedTasksEvent::tasks = tasks;
  // SpecifiedTasksEvent::reset();

  MetricsHub::instance().clearListeners();
  auto csvCollector = std::make_shared<CSVMetricsCollector>(result_file);
  MetricsHub::instance().addListener(csvCollector);

  std::vector<Vehicle::PtrVehicle> vehicles = {
      std::make_shared<Vehicle>(policy),
      std::make_shared<Vehicle>(policy),
      std::make_shared<Vehicle>(policy),
  };
  std::vector<RSU::PtrRSU> rsus = {std::make_shared<RSU>()};
  Simulator sim;

  for (auto v : vehicles) {
    v->set_rsus(rsus);
    v->battery = Battery(10000.0); // Set 10kJ battery
    sim.schedule<TaskGenerationEvent>(1.0, v, Lambda);
  }

  sim.run(duration);
  return 0;
}
