#ifndef BATTERY_H
#define BATTERY_H

#include <algorithm>
#include <iostream>

class Battery {
  double capacity;  // Joules
  double remaining;
  bool infinite = false;

 public:
  Battery(double capacity_j = -1)
      : capacity(capacity_j), remaining(capacity_j) {
    if (capacity_j < 0) infinite = true;  // Use negative for infinite
  }

  void consume(double energy_j) {
    if (infinite) return;
    remaining = std::max(0.0, remaining - energy_j);
  }

  double predict_energy_consumption(double energy_j) {
    if (infinite) return energy_j;
    return remaining - energy_j;
  }

  double get_remaining() const { return remaining; }
  double get_capacity() const { return capacity; }
  double get_percentage() const {
    if (infinite) return 100.0;
    return (capacity > 0) ? (remaining / capacity) * 100.0 : 0.0;
  }

  bool is_depleted() const {
    if (infinite) return false;
    return remaining <= 0.0;
  }

  void set_infinite(bool val) { infinite = val; }
};

#endif  // BATTERY_H
