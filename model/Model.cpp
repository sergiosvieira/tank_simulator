#include "Model.h"
#include "../core/EnergyManager.h"
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

void Model::update_energy(Simulator &sim) {
  double now = sim.now();
  if (last_energy_update <= 0.0001)
    last_energy_update = now; // Init
  double delta = now - last_energy_update;
  if (delta > 0) {
    double idle_power = 2.0; // 2 Watts
    double energy = idle_power * delta;
    battery.consume(energy);
    // We don't report metric here to avoid recursion or log flood
    last_energy_update = now;
  }
}

void Model::report_metric(Simulator &sim, const std::string &name, double value,
                          const std::string &tag, int task_id, const char *file,
                          int line) {
  if (name != "BatteryRemaining")
    update_energy(sim);

  MetricsHub::instance().record(sim.now(), get_id(), name, value, tag, task_id,
                                file, line);
}

// Special report function for origin node metrics
void Model::report_metric_for_node(Simulator &sim, int node_id,
                                   const std::string &name, double value,
                                   const std::string &tag, int task_id,
                                   const char *file, int line) {
  if (node_id == get_id() && name != "BatteryRemaining")
    update_energy(sim);

  MetricsHub::instance().record(sim.now(), node_id, name, value, tag, task_id,
                                file, line);
}

bool Model::accept_processing_task(Simulator &sim, Task::PtrTask task) {
  if (processing_queue.size() < queue_size) {
    processing_queue.push(task);
    if (cpu.is_idle()) {
      schedule_cpu(sim);
    }
    return true;
  }
  return false;
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
  double energy = EnergyManager::calculate_processing_energy(
      cpu.get_freq(), processing_task->total_cycles());
  int origin_id = processing_task->get_origin_node_id();
  if (origin_id == -1)
    origin_id = this->get_id();
  bool was_offloaded = processing_task->get_offloaded();
  int tid = processing_task->get_id();

  if (battery.predict_energy_consumption(energy) < 0.0) {
    report_metric_for_node(sim, origin_id, "TaskSuccess", 0.0, tag, tid);
    report_metric(sim, "LowEnergyFail", 1.0, tag, tid);
    report_metric_for_node(
        sim, origin_id, "OffloadingType", was_offloaded ? 1.0 : 0.0,
        was_offloaded ? "Remote | " + tag : "Local | " + tag, tid);
    return;
  }
  double latency = processing_task->spent_time(sim);
  // Add transfer time for offloaded tasks (network overhead)
  double tx_time = processing_task->get_transfer_time();
  double total_latency = latency + tx_time;
  report_metric_for_node(sim, origin_id, "TaskLatency", total_latency, "", tid);
  bool success = (total_latency <= processing_task->get_deadline());
  report_metric_for_node(sim, origin_id, "TaskSuccess", success ? 1.0 : 0.0,
                         tag, tid);
  double margin = processing_task->get_deadline() - total_latency;
  report_metric_for_node(sim, origin_id, "TaskMargin", margin, tag, tid);

  report_metric_for_node(
      sim, origin_id, "OffloadingType", was_offloaded ? 1.0 : 0.0,
      was_offloaded ? "Remote | " + tag : "Local | " + tag, tid);
  battery.consume(energy);

  report_metric(sim, "EnergyConsumption", energy, "CpuOnly", tid);
  report_metric(sim, "CpuEnergy", energy, tag, tid);
  report_metric(sim, "BatteryRemaining", battery.get_remaining(), tag, tid);

  if (battery.is_depleted()) {
    report_metric(sim, "BatteryDepleted", 1.0, tag, tid);
  }

  processing_task = nullptr;
  if (!processing_queue.empty()) {
    // Check battery before scheduling next?
    if (!battery.is_depleted()) {
      schedule_cpu_start_event(sim);
    }
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
