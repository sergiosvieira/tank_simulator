#include "Simulator.h"

void Simulator::schedule(Event::PtrEvent event) {
  if (event->get_time() < current_time)
    throw std::exception();
  fel.push(event);
}

void Simulator::run(double sim_end_time) {
  while (!fel.empty()) {
    Event::PtrEvent event = fel.top();
    fel.pop();
    if (event->get_time() > sim_end_time)
      break;
    current_time = event->get_time();
    event->execute(*this);
  }
}
