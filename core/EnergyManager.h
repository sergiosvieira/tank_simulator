#ifndef ENERGYMANAGER_H
#define ENERGYMANAGER_H

#include "../utils/Rng.h"
#include "ChaosManager.h"
#include "Config.h"
#include <cmath>

class EnergyManager {
public:
  // Constants for energy model
  static constexpr double K = 1e-28; // Effective switched capacitance

  static double calculate_processing_energy(double frequency, long cycles) {
    double result = K * std::pow(frequency, 2) * cycles;
    if (Config::FIELD_TOTAL_CHAOS) {
      double drift = ChaosManager::instance().get_state();
      return Rng::pdrift(result, drift);
    }
    return result;
  }

  // Placeholder for future transmission energy logic
  static double calculate_transmission_energy(double size_bytes,
                                              double distance) {
    // Linear model: 5.0 Joules per MB (High Tx power for long range)
    double result = 5.0 * size_bytes / 1e6;
    if (Config::FIELD_TOTAL_CHAOS) {
      double drift = ChaosManager::instance().get_state();
      return Rng::pdrift(result, drift);
    }
    return result;
  }
};

#endif // ENERGYMANAGER_H
