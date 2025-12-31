#include "TaskGenerationEvent.h"
#include "../core/ChaosManager.h"
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

  // Calculate inter-arrival time
  double inter_arrival = Rng::exponential(lambda);

  // Non-stationary arrivals in chaos mode
  if (Config::FIELD_TOTAL_CHAOS) {
    double drift = ChaosManager::instance().get_state();
    inter_arrival = Rng::pdrift(inter_arrival, drift);
  }

  double next_event = sim.now() + inter_arrival;
  sim.schedule(
      std::make_shared<TaskGenerationEvent>(next_event, model, lambda));
}
