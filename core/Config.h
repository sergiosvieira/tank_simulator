#ifndef CONFIG_H
#define CONFIG_H

namespace Config {

// --- Traffic Generation ---
// Lambda = Arrival Rate (tasks per second)
// 1 task every 0.08s => 12.5 tasks/s
static constexpr double TRAFFIC_LAMBDA = 1.0 / 0.08;

// --- Task Characteristics ---
static constexpr long TASK_MIN_SIZE = 100000; // 100 KB
static constexpr long TASK_MAX_SIZE = 300000; // 300 KB

static constexpr long TASK_MEAN_DENSITY = 1000; // 1000 cycles/byte
static constexpr long TASK_STD_DENSITY = 100;   // 100 cycles/byte

static constexpr double TASK_MIN_DEADLINE = 0.4; // 400 ms
static constexpr double TASK_MAX_DEADLINE = 0.5; // 500 ms

// --- Uncertainty Reference Bounds (Normalization Factors) ---
// Used for entropy calculation relative to a "maximum expected" design space
static constexpr double REF_ARRIVAL_MAX_MEAN =
    1.0 / 0.01; // Max Mean Inter-arrival: 100s
static constexpr double REF_SIZE_SPAN = 1000000.0;    // 1 MB range
static constexpr double REF_DENSITY_STD_MAX = 1000.0; // High variance
static constexpr double REF_DEADLINE_SPAN = 10.0;     // 10s range

} // namespace Config

#endif // CONFIG_H
