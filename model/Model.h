#ifndef MODEL_H
#define MODEL_H

#include "CPU.h"
#include "EventType.h"
#include "Task.h"
#include "core/Simulator.h"
#include "utils/IdManager.h"
#include <functional>
#include <memory>
#include <queue>
#include <string>
#include <unordered_map>

class Model : public std::enable_shared_from_this<Model> {
protected:
  int id = IdManager::next_id();
  std::queue<Task::PtrTask> processing_queue;
  Task::PtrTask processing_task = nullptr;

public:
  using PtrModel = std::shared_ptr<Model>;
  using Map = std::unordered_map<EventType, std::function<void(Simulator &)>>;

  CPU cpu;
  Map events;

  virtual ~Model() = default;

  Model();

  int get_id() const { return id; }

  void report_metric(Simulator &sim, const std::string &name, double value,
                     const std::string &tag = "");

  // Accepts task for PROCESSING
  virtual void accept_processing_task(Simulator &sim, Task::PtrTask task);

  void OnProcessingStart(Simulator &sim);
  void OnProcessingComplete(Simulator &sim);

protected:
  virtual void schedule_cpu(Simulator &sim);
  virtual void schedule_processing_complete(Simulator &sim);
  virtual void schedule_cpu_start_event(Simulator &sim);
};

#endif // MODEL_H
