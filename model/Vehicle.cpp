#include "Vehicle.h"
#include "../events/DecisionEvent.h"
#include "../logger.h"
#include <sstream>

Vehicle::Vehicle(OffPolicy::PtrOffPolicy policy) : Model() {
  tag = policy->get_name();
  off_policy = policy;
  // off_policy->set_host(shared_from_this()); // THIS IS THE BUG! Cannot call
  // shared_from_this in constructor!
  events[EventType::OnDecisionStart] = [this](Simulator &sim) {
    this->onDecisionStart(sim);
  };
  events[EventType::OnDecisionComplete] = [this](Simulator &sim) {
    this->onDecisionComplete(sim);
  };
}

void Vehicle::set_rsus(const std::vector<RSU::PtrRSU> &rsus_) {
  rsus = rsus_;
  // Set host here, where shared_from_this() is valid
  if (off_policy) {
    off_policy->set_host(shared_from_this());
  }
}

void Vehicle::add_task_to_decision(Simulator &sim, Task::PtrTask task) {
  decision_queue.push(task);
  if (off_policy->is_idle()) {
    schedule_decision_start_event(sim);
  }
}

void Vehicle::onDecisionStart(Simulator &sim) {
  if (off_policy->is_idle() && !decision_queue.empty()) {
    report_metric(sim, "QueueSize_Decision", (double)decision_queue.size());
    decision_task = decision_queue.front();
    decision_queue.pop();
    off_policy->start();
    std::stringstream ss;
    ss << "Task " << decision_task->get_id() << " | Node " << this->get_id()
       << " | DECISION_START"
       << " | queue_size=" << decision_queue.size();
    LOG_INFO(sim.now(), ss.str());

    schedule_decision(sim);
  }
}

void Vehicle::onDecisionComplete(Simulator &sim) {
  std::stringstream ss;
  ss << "Task " << decision_task->get_id() << " | Node " << this->get_id()
     << " | DECISION_COMPLETE";
  auto result = off_policy->decide(decision_task, rsus);
  ss << " | decision="
     << (result.decision_type == DecisionType::Local ? "Local" : "Remote")
     << " | destiny=Node "
     << (result.choosed_device != nullptr ? result.choosed_device->get_id()
                                          : get_id());
  LOG_INFO(sim.now(), ss.str());
  off_policy->complete();

  if (result.decision_type == DecisionType::Local) {
    // Local processing: call Base implementation
    bool accepted = this->accept_processing_task(sim, decision_task);
    if (!accepted)
      report_metric_for_node(sim, get_id(), "FullQueueError", 1.0, "Local");
  } else {
    // Remote processing
    decision_task->set_offloaded(true);
    if (result.choosed_device) {
      bool accepted =
          result.choosed_device->accept_processing_task(sim, decision_task);
      if (!accepted)
        report_metric_for_node(sim, result.choosed_device->get_id(),
                               "FullQueueError", 1.0, "Remote");
    } else {
      // Fallback if no device chosen? For now Local.
      bool accepted = this->accept_processing_task(sim, decision_task);
      if (!accepted)
        report_metric_for_node(sim, get_id(), "FullQueueError", 1.0,
                               "Local|Fallback");
    }
  }

  decision_task = nullptr;
  if (!decision_queue.empty()) {
    schedule_decision_start_event(sim);
  }
}

void Vehicle::schedule_decision(Simulator &sim) {
  sim.schedule<DecisionEvent>(
      sim.now() + off_policy->decision_time(decision_task),
      std::static_pointer_cast<Vehicle>(shared_from_this()),
      EventType::OnDecisionComplete);
}

void Vehicle::schedule_decision_start_event(Simulator &sim) {
  sim.schedule<DecisionEvent>(
      sim.now(), std::static_pointer_cast<Vehicle>(shared_from_this()),
      EventType::OnDecisionStart);
}
