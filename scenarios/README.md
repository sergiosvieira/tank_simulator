# Manual Scenarios Documentation

## Overview

This folder contains infrastructure for creating and running **deterministic/manual test scenarios**.

Unlike the standard simulation which uses stochastic policies, here you have **complete control** over:
- Which tasks are generated
- When they arrive
- How big they are
- What the deadline is
- **Where each task goes (local vs remote)**

## Files

| File | Description |
|------|-------------|
| `Scenario.h` | Base class for all scenarios |
| `OracleScenario.h` | Optimal (Oracle) scenario + StressTest + SimpleValidation |

## Usage

### Command Line

```bash
# Build and run a scenario
./run_scenarios.sh <scenario_name> [--chaos] [seed]

# Examples:
./run_scenarios.sh Simple          # Basic validation
./run_scenarios.sh Oracle          # Optimal decisions (normal)
./run_scenarios.sh Oracle --chaos  # Optimal decisions (chaos mode)
./run_scenarios.sh StressTest      # Extreme load test
```

### Available Scenarios

| Scenario | Description | Expected Success |
|----------|-------------|------------------|
| `Simple` | 3 basic tasks for validation | 100% |
| `Oracle` | 23 tasks with optimal decisions | 95%+ |
| `StressTest` | 20 burst tasks, extreme load | 70% |

## Creating Custom Scenarios

### 1. Create a new scenario class

```cpp
#include "Scenario.h"

class MyCustomScenario : public Scenario {
public:
    std::string name() const override {
        return "MyCustom";
    }
    
    std::string description() const override {
        return "My custom test scenario";
    }
    
    std::vector<ScenarioTask> build_tasks() const override {
        std::vector<ScenarioTask> tasks;
        
        // ScenarioTask(timestamp, size_bytes, density, deadline, decision)
        
        // Task 1: Local processing
        tasks.push_back({1.0, 100000, 1000, 0.5, ScriptedDecision::Local()});
        
        // Task 2: Offload to RSU 0
        tasks.push_back({2.0, 250000, 1200, 1.0, ScriptedDecision::Remote(0)});
        
        // Task 3: Local
        tasks.push_back({3.0, 80000, 900, 0.4, ScriptedDecision::Local()});
        
        return tasks;
    }
    
    double expected_success_rate() const override {
        return 0.95;  // 95%
    }
};
```

### 2. Register in run_scenario.cpp

```cpp
std::unique_ptr<Scenario> create_scenario(const std::string& name) {
    // ... existing scenarios ...
    
    if (name == "MyCustom") {
        return std::make_unique<MyCustomScenario>();
    }
    
    return nullptr;
}
```

## DeterministicPolicy

The `DeterministicPolicy` class (in `model/DeterministicPolicy.h`) allows scripted decision-making:

```cpp
auto policy = std::make_shared<DeterministicPolicy>();

// Add individual decisions
policy->add_decision(task_id, ScriptedDecision::Local());
policy->add_decision(task_id, ScriptedDecision::Remote(0));

// Or set ranges
policy->set_all_local(1, 10);      // Tasks 1-10: local
policy->set_all_remote(11, 20, 0); // Tasks 11-20: RSU 0
```

## Oracle Scenario Details

The Oracle scenario is designed to demonstrate the **best possible decisions** under chaos:

### Phase 1: Normal Load (t=1-5s)
- Small tasks → Local (minimize overhead)
- Large tasks → Remote (offload heavy work)

### Phase 2: Burst/Chaos (t=5-10s)
- Heavy burst → Aggressive offloading
- Critical tight-deadline → Local for speed

### Phase 3: Recovery (t=10-15s)
- Mixed decisions as load decreases

## Results

Results are saved to:
```
results/scenario_<name>_<seed>.csv
```

Columns include:
- TaskLatency
- TaskSuccess
- TaskMargin (time remaining before deadline)
- OffloadingType
- EnergyConsumption

## Comparing with Other Policies

To compare Oracle with other policies under chaos:

```bash
# Oracle (optimal)
./run_scenarios.sh Oracle --chaos 42

# Run standard simulation with other policies
./tank_simulator Local 30 42 --chaos
./tank_simulator Random 30 42 --chaos
./tank_simulator Intelligent 30 42 --chaos
```

Then compare the success rates from each CSV.
