#ifndef ENERGYMANAGER_H
#define ENERGYMANAGER_H

#include <cmath>

class EnergyManager {
public:
  // Constants for energy model
  static constexpr double K = 1e-28; // Effective switched capacitance

  static double calculate_processing_energy(double frequency, long cycles) {
    return K * std::pow(frequency, 2) * cycles;
  }

  // Placeholder for future transmission energy logic
  static double calculate_transmission_energy(double size_bytes,
                                              double distance) {
    // Simple linear model example: 100mW base + distance factor
    return 0.1 * size_bytes / 1e6;
  }
};

#endif // ENERGYMANAGER_H
