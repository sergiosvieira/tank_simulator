#ifndef RNG_H
#define RNG_H

#include <random>
#include <stdexcept>
#include <vector>

// Forward declaration - ChaosManager is now in core/ChaosManager.h
class ChaosManager;

struct Rng {
  static constexpr int seed = 1978;
  static std::mt19937 &engine() {
    static std::mt19937 e{seed};
    return e;
  }

  static double exponential(double lambda) {
    std::exponential_distribution<double> d(lambda);
    return d(engine());
  }

  static double normal(double mu, double sigma) {
    std::normal_distribution<double> d(mu, sigma);
    return d(engine());
  }

  static double uniform(double a, double b) {
    std::uniform_real_distribution<double> d(a, b);
    return d(engine());
  }

  static int uniform_int(int a, int b) {
    std::uniform_int_distribution<int> d(a, b);
    return d(engine());
  }

  /**
   * @brief Apply drift to a base value using explicit drift state
   *
   * This is the low-level function that applies the multiplicative drift.
   * For consistent chaos across the system, use
   * ChaosManager::instance().apply_drift()
   *
   * @param base The base value to perturb
   * @param drift_state The drift factor (typically from ChaosManager.z)
   * @return Perturbed value, guaranteed >= epsilon
   */
  static double pdrift(double base, double drift_state) {
    static constexpr double epsilon = 1e-6;
    return std::max(base * (1.0 + drift_state), epsilon);
  }

  template <typename T> static T sample(const std::vector<T> &vec) {
    if (vec.empty()) {
      throw std::runtime_error(
          "Rng::sample: Attempted to sample from an empty vector.");
    }
    std::uniform_int_distribution<size_t> d(0, vec.size() - 1);
    return vec[d(engine())];
  }
};

#endif // RNG_H
