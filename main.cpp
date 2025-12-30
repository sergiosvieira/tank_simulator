#include "core/Simulator.h"
#include "events/TaskGenerationEvent.h"
#include "logger.h"
#include "metric.h"
#include "model/RSU.h"
#include "model/Vehicle.h"
#include "utils/Rng.h"
#include "model/RandomPolicy.h"
#include <iostream>
#include <memory>
#include <vector>

using std::cout, std::endl;

int main() {
    std::vector<std::string> results = {
        "results/experiment_001_local.csv",
        "results/experiment_001_random.csv"
    };
    std::vector<OffPolicy::PtrOffPolicy> policies = {
        std::make_shared<OffPolicy>(),
        std::make_shared<RandomPolicy>()
    };
    for (size_t i = 0; i < results.size(); ++i) {
        auto csvCollector =
          std::make_shared<CSVMetricsCollector>(results[i]);
        MetricsHub::instance().addListener(csvCollector);

        std::vector<Vehicle::PtrVehicle> vehicles = {
            std::make_shared<Vehicle>(policies[i]),
        };
        std::vector<RSU::PtrRSU> rsus = {std::make_shared<RSU>()};
        Simulator sim;
        for (auto v : vehicles) {
        v->set_rsus(rsus);
        sim.schedule<TaskGenerationEvent>(Rng::uniform(0.0, 0.01), v,
                                          1.0 / Rng::uniform(0.02, 0.05));
        }
        sim.run(500);
    }
  return 0;
}
