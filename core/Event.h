#ifndef EVENT_H
#define EVENT_H

#include <memory>
#include <vector>

class Simulator;

class Event {
protected:
  double time = 0.0;
public:
  using PtrEvent = std::shared_ptr<Event>;
  using VecPtrEvent = std::vector<PtrEvent>;
  Event(double time_) : time(time_) {}
  virtual ~Event() = default;
  double get_time() const { return time; }
  virtual void execute(Simulator &sim) = 0;
  bool operator>(const Event &other) const { return time > other.time; }
};

struct EventPtrComparator {
  bool operator()(const Event::PtrEvent &lhs, const Event::PtrEvent &rhs) {
    return *lhs > *rhs;
  }
};

#endif // EVENT_H
