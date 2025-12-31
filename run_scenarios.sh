#!/bin/bash
# Build and run scenario tests
# Usage: ./run_scenarios.sh [scenario_name] [--chaos]

set -e

SCENARIO=${1:-Oracle}
CHAOS_FLAG=${2:-}

echo "=========================================="
echo "Building run_scenario..."
echo "=========================================="

# Compile run_scenario
g++ -std=gnu++1z -O2 -I. -o run_scenario \
    run_scenario.cpp \
    core/Simulator.cpp \
    core/Config.cpp \
    events/SpecifiedTasksEvent.cpp \
    events/TaskGenerationEvent.cpp \
    model/CPU.cpp \
    model/Model.cpp \
    model/OffPolicy.cpp \
    model/RandomPolicy.cpp \
    model/FirstRemotePolicy.cpp \
    model/IntelligentPolicy.cpp \
    model/Task.cpp \
    model/Vehicle.cpp \
    -Wall -Wextra

echo "Build successful!"
echo ""

echo "=========================================="
echo "Running scenario: $SCENARIO $CHAOS_FLAG"
echo "=========================================="

./run_scenario $SCENARIO $CHAOS_FLAG

echo ""
echo "Done!"
