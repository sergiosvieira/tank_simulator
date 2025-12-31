#include "SpecifiedTasksEvent.h"
#include "../logger.h"
#include "../model/Task.h"
#include "../utils/Rng.h"
#include <sstream>

size_t SpecifiedTasksEvent::index = 0;
std::vector<Task::PtrTask> SpecifiedTasksEvent::tasks = {};

void SpecifiedTasksEvent::execute(Simulator &sim) {
  if (index < tasks.size()) {
    if (!tasks[index]) {
      return;
    }
    Task::PtrTask task = tasks[index++];
    task->set_origin_node_id(model->get_id()); // Set origin
    model->report_metric(sim, "TaskTotalCycles", task->total_cycles(), "",
                         task->get_id());
    std::stringstream ss;
    ss << "Task " << task->get_id() << " | Node " << model->get_id()
       << " | TASK_GENERATED"
       << " | data_size=" << task->get_data_size()
       << " | cycles per byte=" << task->get_cycles()
       << " | deadline=" << task->get_deadline();
    LOG_INFO(sim.now(), ss.str());

    // Push to decision queue
    model->add_task_to_decision(sim, task);
    if (index < tasks.size()) {
      sim.schedule<SpecifiedTasksEvent>(tasks[index]->get_timestamp(), model);
    } else {
      sim.schedule<SpecifiedTasksEvent>(sim.get_end_time() + 1, model);
    }
  } else {
    sim.schedule<SpecifiedTasksEvent>(sim.get_end_time() + 1, model);
  }
}
