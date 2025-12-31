#pragma once

/**
 * @brief Global Chaos State Manager
 *
 * Models environmental uncertainty as a latent global chaos state shared across
 * all task and network parameters, evolving as a temporally correlated
 * stochastic process.
 *
 * Mathematical Model (AR(1) Process):
 *   z_t = ρ * z_{t-1} + σ * ε_t
 *
 * Where:
 *   - ρ ∈ (0,1) = memory/persistence coefficient (default: 0.9)
 *   - σ = chaos intensity (from Config::CHAOS_INTENSITY)
 *   - ε_t ~ U(-1, 1) = uniform noise
 *   - z_t = chaos state at time t
 *
 * Properties:
 *   ✔️ Temporal correlation (memory)
 *   ✔️ Mean zero (long-term)
 *   ✔️ Bounded variance
 *   ✔️ Shared across ALL parameters (structural correlation)
 */

#include "Config.h"
#include <algorithm>
#include <random>

class ChaosManager {
public:
  // AR(1) persistence coefficient (0 < ρ < 1)
  // Higher = more temporal correlation (more "sticky" chaos)
  static constexpr double RHO = 0.9;

  // Chaos state z_t ∈ ℝ
  double z = 0.0;

  /**
   * @brief Update the chaos state (call this at each event or fixed Δt)
   *
   * Implements: z_t = ρ * z_{t-1} + σ * ε_t
   * where ε_t ~ U(-1, 1)
   */
  void update() {
    std::uniform_real_distribution<double> noise_dist(-1.0, 1.0);
    double epsilon = noise_dist(chaos_engine_);
    z = RHO * z + Config::CHAOS_INTENSITY * epsilon;
  }

  /**
   * @brief Apply chaos drift to a base value
   *
   * Formula: result = max(base * (1 + z), ε)
   * Ensures positive values with minimum epsilon.
   *
   * @param base The original parameter value
   * @return The perturbed value
   */
  double apply_drift(double base) const {
    static constexpr double EPSILON = 1e-6;
    return std::max(base * (1.0 + z), EPSILON);
  }

  /**
   * @brief Get the current chaos state value
   * @return Current chaos intensity z_t
   */
  double get_state() const { return z; }

  /**
   * @brief Reset chaos state to zero
   */
  void reset() { z = 0.0; }

  /**
   * @brief Seed the internal chaos RNG for reproducibility
   * @param seed The random seed
   */
  void seed(int s) { chaos_engine_.seed(s); }

  // Singleton access
  static ChaosManager &instance() {
    static ChaosManager manager;
    return manager;
  }

private:
  ChaosManager() : chaos_engine_(42) {}

  // Internal RNG engine (separate from main simulation RNG)
  std::mt19937 chaos_engine_;
};
