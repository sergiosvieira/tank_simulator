#pragma once

namespace Config {

// --------------------------------------------------
// Chaos Mode
// --------------------------------------------------

inline bool FIELD_TOTAL_CHAOS = false;

// --------------------------------------------------
// Chaos Intensity (temporal amplification)
// --------------------------------------------------
inline double CHAOS_INTENSITY = 1.0; // default: no amplification

inline void set_chaos_mode() {
  FIELD_TOTAL_CHAOS = true;
  CHAOS_INTENSITY = 2.5; // moderate chaos (was 2.5, reduced for stability)
}

// --------------------------------------------------
// Traffic Generation
// --------------------------------------------------
// Lambda = Arrival Rate (tasks per second)
// 1 task every 0.08s => 12.5 tasks/s
inline double TRAFFIC_LAMBDA = 1.0 / 0.08;

// --------------------------------------------------
// Task Characteristics
// --------------------------------------------------
inline long TASK_MIN_SIZE = 100000;
inline long TASK_MAX_SIZE = 300000;

inline long TASK_MEAN_DENSITY = 1000;
inline long TASK_STD_DENSITY = 100;

inline double TASK_MIN_DEADLINE = 0.4;
inline double TASK_MAX_DEADLINE = 0.5;

// --------------------------------------------------
// Uncertainty Reference Bounds (FIXED DESIGN SPACE)
// --------------------------------------------------
// Used for entropy calculation relative to a "maximum expected" design space
static constexpr double REF_ARRIVAL_MAX_MEAN = 1.0 / 0.01; // 100s
static constexpr double REF_SIZE_SPAN = 1000000.0;         // 1 MB
static constexpr double REF_DENSITY_STD_MAX = 1000.0;
static constexpr double REF_DEADLINE_SPAN = 10.0;

} // namespace Config
