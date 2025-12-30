#include "TaskGenerationEvent.h"
#include "../logger.h"
#include "../model/Task.h"
#include "../utils/Rng.h"
#include <sstream>

void TaskGenerationEvent::execute(Simulator &sim) {
  Task::PtrTask task = std::make_shared<Task>(sim);
  model->report_metric(sim, "TaskTotalCycles", task->total_cycles());
  std::stringstream ss;
  ss << "Task " << task->get_id() << " | Node " << model->get_id()
     << " | TASK_GENERATED"
     << " | deadline=" << task->get_deadline();
  LOG_INFO(sim.now(), ss.str());

  // Push to decision queue
  model->add_task_to_decision(sim, task);

  double next_gen_event = Rng::exponential(lambda);
  // sim.schedule via template requires type.
  // We can't use template inside .cpp easily if we don't include sim definition
  // (which we do via include chain) BUT the template schedule<T> is in
  // Simulator.h. So we need Simulator.h included. `Event.h` only fwds it.
  // `TaskGenerationEvent.h` includes `Event.h`.
  // `Vehicle.h` includes `Model.h` which includes `Simulator.h`.
  // So `Simulator.h` is available via `Vehicle.h`.
  // But relying on transitive includes is brittle. Start with Simulator.h
  // Wait, `Event.h` only forward declares Simulator.
  // `TaskGenerationEvent.h` includes `Event.h`. And `Vehicle.h`.
  // `Vehicle.h` -> `Model.h` -> `Simulator.h`.
  // So we have definition of Simulator.

  sim.schedule<TaskGenerationEvent>(sim.now() + next_gen_event, model, lambda);
}
