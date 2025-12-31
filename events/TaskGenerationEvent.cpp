#include "TaskGenerationEvent.h"
#include "../logger.h"
#include "../model/Task.h"
#include "../utils/Rng.h"
#include <sstream>

void TaskGenerationEvent::execute(Simulator &sim) {
  Task::PtrTask task = std::make_shared<Task>(sim);
  task->set_origin_node_id(model->get_id()); // Set origin
  model->report_metric(sim, "TaskTotalCycles", task->total_cycles());
  std::stringstream ss;
  ss << "Task " << task->get_id() << " | Node " << model->get_id()
     << " | TASK_GENERATED"
     << " | lambda=" << lambda << " | deadline=" << task->get_deadline();
  LOG_INFO(sim.now(), ss.str());

  // Push to decision queue
  model->add_task_to_decision(sim, task);

  // Non-stationary arrivals in chaos mode
  if (Config::FIELD_TOTAL_CHAOS) {
    static double last_lambda = Config::TRAFFIC_LAMBDA;

    // Regime switching (aggressive)
    double multiplier = Rng::uniform(0.5, 2.0);
    lambda = last_lambda * multiplier;

    // Rare burst shock (5% probability)
    if (Rng::uniform(0.0, 1.0) < 0.05) {
      lambda *= 5.0; // burst amplification (reduced from 10x for stability)
    }

    // Clamp to avoid numerical explosion (reduced from 50x for stability)
    double max_lambda = 30.0 * Config::TRAFFIC_LAMBDA; // ~375 tasks/s max
    if (lambda > max_lambda)
      lambda = max_lambda;
    if (lambda < 1.0)
      lambda = 1.0;

    last_lambda = lambda;
  }

  double inter_arrival = Rng::exponential(lambda);

  // CRITICAL: Enforce minimum inter-arrival time to prevent double precision
  // issues when current_time is large (e.g., 170.17575691181059 + 1e-16 = same
  // value) This prevents temporal causality violations (scheduling events in
  // the past)
  constexpr double MIN_INTER_ARRIVAL = 1e-9; // 1 nanosecond
  if (inter_arrival < MIN_INTER_ARRIVAL) {
    inter_arrival = MIN_INTER_ARRIVAL;
  }

  sim.schedule(std::make_shared<TaskGenerationEvent>(sim.now() + inter_arrival,
                                                     model, lambda));
}
