#include "core/Simulator.h"
#include "events/TaskGenerationEvent.h"
#include "logger.h"
#include "metric.h"
#include "model/RSU.h"
#include "model/Vehicle.h"
#include "utils/Rng.h"
#include <iostream>
#include <memory>
#include <vector>

using std::cout, std::endl;

int main() {
  auto csvCollector =
      std::make_shared<CSVMetricsCollector>("results/experiment_001.csv");
  MetricsHub::instance().addListener(csvCollector);
  std::vector<Vehicle::PtrVehicle> vehicles = {
      std::make_shared<Vehicle>(),
      // std::make_shared<Vehicle>(),
      // std::make_shared<Vehicle>()
  };
  std::vector<RSU::PtrRSU> rsus = {std::make_shared<RSU>()};
  Simulator sim;
  for (auto v : vehicles) {
    sim.schedule<TaskGenerationEvent>(Rng::uniform(0.0, 0.01), v,
                                      1.0 / Rng::uniform(0.02, 0.1));
  }
  sim.run(1000);
  return 0;
}
