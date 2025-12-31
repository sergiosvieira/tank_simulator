#!/bin/bash

# Configuration
POLICIES=("Local" "Random" "Intelligent" "FirstRemote")
DURATION=500
REPEATS=5
BASE_SEED=1978

# Create results directory if it doesn't exist
mkdir -p results

# Compile Simulator to ensure everything is up to date
echo "Compiling Simulator..."
make -j4

echo "Starting Simulation Campaign..."
echo "Policies: ${POLICIES[*]}"
echo "Repeats: $REPEATS"
echo "Duration: $DURATION s"

# Iterate over each policy
for policy in "${POLICIES[@]}"; do
    echo "------------------------------------------------"
    echo "Running policy: $policy"
    echo "------------------------------------------------"
    
    # Run repeats
    for i in $(seq 1 $REPEATS); do
        # Generate a unique seed for this run
        SEED=$((BASE_SEED + i))
        
        # Run simulation in background to speed up (optional)
        # Remove '&' and 'wait' logic if you prefer sequential execution
        ./tank_simulator "$policy" "$DURATION" "$SEED" > /dev/null &
        
        # Limit parallel jobs to avoid overloading (e.g., max 8 parallel)
        if (( i % 8 == 0 )); then
            wait
        fi
    done
    
    # Wait for remaining jobs of this policy
    wait
    echo "Finished $policy."
done

echo "------------------------------------------------"
echo "Campaign Completed!"

# --- POST-PROCESSING ---

echo "------------------------------------------------"
echo "Starting Post-Processing Pipeline..."

# 1. Compile Aggregator (Fast C++ Tool)
if [ ! -f "tools/log_aggregator" ] || [ "tools/LogAggregator.cpp" -nt "tools/log_aggregator" ]; then
    echo "Compiling C++ Log Aggregator..."
    mkdir -p tools
    g++ -O3 -std=c++17 -pthread -o tools/log_aggregator tools/LogAggregator.cpp
fi

# 2. Run Aggregation
echo "Aggregating Logs (C++)..."
./tools/log_aggregator results/

# 3. Generate Dashboard (Python)
echo "Generating Dashboard..."
PYTHON_EXEC="/home/sergiosvieira/miniconda3/envs/simulator/bin/python"

if [ -f "$PYTHON_EXEC" ]; then
    $PYTHON_EXEC scripts/dashboard_fast.py
else
    # Fallback to system python if conda env not found (though user said to always use it)
    python scripts/dashboard_fast.py
fi

echo "Pipeline Finished. Check FINAL_DASHBOARD_FAST.png"
