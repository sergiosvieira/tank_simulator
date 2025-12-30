#include "Model.h"
#include "../events/CPUEvent.h"
#include "../logger.h"
#include "../metric.h"
#include <cmath>
#include <sstream>

Model::Model() {
  events[EventType::OnProcessingStart] = [this](Simulator &sim) {
    this->OnProcessingStart(sim);
  };
  events[EventType::OnProcessingComplete] = [this](Simulator &sim) {
    this->OnProcessingComplete(sim);
  };
}

void Model::report_metric(Simulator &sim, const std::string &name, double value,
                          const std::string &tag) {
  RECORD_METRIC(sim, this->get_id(), name, value, tag);
}

void Model::accept_processing_task(Simulator &sim, Task::PtrTask task) {
  processing_queue.push(task);
  if (cpu.is_idle()) {
    schedule_cpu(sim);
  }
}

void Model::OnProcessingStart(Simulator &sim) {
  if (cpu.is_idle() && !processing_queue.empty()) {
    processing_task = processing_queue.front();
    processing_queue.pop();
    std::stringstream ss;
    ss << "Task " << processing_task->get_id() << " | Node " << this->get_id()
       << " | PROCESSING_START"
       << " | queue_size=" << processing_queue.size();
    LOG_INFO(sim.now(), ss.str());
    cpu.start();
    schedule_processing_complete(sim);
  }
}

void Model::OnProcessingComplete(Simulator &sim) {
  std::stringstream ss;
  ss << "Task " << processing_task->get_id() << " | Node " << this->get_id()
     << " | PROCESSING_COMPLETE"
     << " | completion_time=" << processing_task->spent_time(sim)
     << " | offloaded=" << (processing_task->get_offloaded() ? "Yes" : "No")
     << " | success="
     << (processing_task->spent_time(sim) < processing_task->get_deadline()
             ? "Yes"
             : "No");
  LOG_INFO(sim.now(), ss.str());
  cpu.complete();

  double latency = processing_task->spent_time(sim);
  report_metric(sim, "TaskLatency", latency);
  bool success = (latency <= processing_task->get_deadline());
  report_metric(sim, "TaskSuccess", success ? 1.0 : 0.0);
  bool was_offloaded = processing_task->get_offloaded();
  report_metric(sim, "OffloadingType", was_offloaded ? 1.0 : 0.0,
                was_offloaded ? "Remote" : "Local");
  double energy =
      1e-28 * std::pow(cpu.get_freq(), 2) * processing_task->total_cycles();
  report_metric(sim, "EnergyConsumption", energy);

  processing_task = nullptr;
  if (!processing_queue.empty()) {
    schedule_cpu_start_event(sim);
  }
}

void Model::schedule_cpu(Simulator &sim) {
  sim.schedule<CPUEvent>(sim.now(), shared_from_this(),
                         EventType::OnProcessingStart);
}
void Model::schedule_processing_complete(Simulator &sim) {
  sim.schedule<CPUEvent>(sim.now() + cpu.processing_time(processing_task),
                         shared_from_this(), EventType::OnProcessingComplete);
}
void Model::schedule_cpu_start_event(Simulator &sim) {
  sim.schedule<CPUEvent>(sim.now() + micro_step, shared_from_this(),
                         EventType::OnProcessingStart);
}
