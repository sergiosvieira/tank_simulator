#include "Simulator.h"

void Simulator::schedule(Event::PtrEvent event) {
  if (event->get_time() < current_time) {
    std::cerr << "ERROR: Temporal causality violation!\n"
              << "  Trying to schedule event at t=" << event->get_time()
              << " but current_time=" << current_time << "\n"
              << "  Delta: " << (event->get_time() - current_time) << "\n";
    throw std::runtime_error("Cannot schedule event in the past");
  }
  fel.push(event);
}

void Simulator::run(double sim_end_time) {
  end_time = sim_end_time;
  while (!fel.empty()) {
    Event::PtrEvent event = fel.top();
    fel.pop();
    if (event->get_time() > sim_end_time)
      break;
    current_time = event->get_time();
    event->execute(*this);
  }
}
