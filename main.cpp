#include "core/Simulator.h"
#include "events/TaskGenerationEvent.h"
#include "logger.h"
#include "metric.h"
#include "model/IntelligentPolicy.h"
#include "model/RSU.h"
#include "model/RandomPolicy.h"
#include "model/FirstRemotePolicy.h"
#include "model/Vehicle.h"
#include "utils/Rng.h"
#include <iostream>
#include <memory>
#include <vector>

using std::cout, std::endl;

int main() {
  std::vector<std::string> results = {
    "results/experiment_001_local.csv",
    "results/experiment_001_random.csv",
    "results/experiment_001_intelligent.csv",
    "results/experiment_001_first_remote.csv"
  };
  std::vector<OffPolicy::PtrOffPolicy> policies = {
      std::make_shared<OffPolicy>(), std::make_shared<RandomPolicy>(),
      std::make_shared<IntelligentPolicy>(),
      std::make_shared<FirstRemotePolicy>()
  };
  for (size_t i = 0; i < results.size(); ++i) {
    MetricsHub::instance().clearListeners();
    auto csvCollector = std::make_shared<CSVMetricsCollector>(results[i]);
    MetricsHub::instance().addListener(csvCollector);

    std::vector<Vehicle::PtrVehicle> vehicles = {
        std::make_shared<Vehicle>(policies[i]),
    };
    std::vector<RSU::PtrRSU> rsus = {std::make_shared<RSU>()};
    Simulator sim;
    for (auto v : vehicles) {
      v->set_rsus(rsus);
      sim.schedule<TaskGenerationEvent>(Rng::uniform(0.0, 0.01), v, Lambda);
    }
    sim.run(100);
  }
  return 0;
}
