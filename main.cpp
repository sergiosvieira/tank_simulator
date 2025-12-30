#include <iostream>
#include <queue>
#include <vector>
#include <memory>
#include <random>
#include <armadillo>
#include <unordered_map>
#include <functional>
#include "logger.h"
#include "metric.h"

using std::cout, std::endl;
using namespace arma;

// ********** Utilitários **********

struct Rng {
    static constexpr int seed = 1978;
    static std::mt19937& engine() {
        static std::mt19937 e{seed};
        return e;
    }

    static double exponential(double lambda) {
        std::exponential_distribution<double> d(lambda);
        return d(engine());
    }

    static double normal(double mu, double sigma) {
        std::normal_distribution<double> d(mu, sigma);
        return d(engine());
    }

    static double uniform(double a, double b) {
        std::uniform_real_distribution<double> d(a, b);
        return d(engine());
    }

    static int uniform_int(int a, int b) {
        std::uniform_int_distribution<int> d(a, b);
        return d(engine());
    }
};

struct IdManager {
    static int next_id() {
        static int id = 0;
        return ++id;
    }
};

// ********** Específico do Discrete Event Simulator

const double micro_step = 0.0001;

class Simulator;

class Event {
    double time = 0.0;
public:
    using PtrEvent = std::shared_ptr<Event>;
    using VecPtrEvent = std::vector<PtrEvent>;
    Event(double time_): time(time_) {}
    ~Event() = default;
    double get_time() const {
        return time;
    }
    virtual void execute(Simulator& sim) = 0;
    bool operator>(const Event& other) const {
        return time > other.time;
    }
};

struct EventPtrComparator {
    bool operator()(const Event::PtrEvent& lhs,
                    const Event::PtrEvent& rhs) {
        return *lhs > *rhs;
    }
};

class Simulator {
    std::priority_queue<Event::PtrEvent,
                        Event::VecPtrEvent,
                        EventPtrComparator> fel;
    double current_time = 0.0;
public:
    double now() const {
        return current_time;
    }
    void schedule(Event::PtrEvent event) {
        if (event->get_time() < current_time) throw std::exception();
        fel.push(event);
    }
    template<typename T, typename... Args>
    void schedule(double t, Args&&... args) {
        schedule(std::make_shared<T>(t, std::forward<Args>(args)...));
    }
    void run(double sim_end_time) {
        while (!fel.empty()) {
            Event::PtrEvent event = fel.top();
            fel.pop();
            if (event->get_time() > sim_end_time) break;
            current_time = event->get_time();
            event->execute(*this);
        }
    }
};

// ********** Específico do Task Offloading **********

class Task {
    double timestamp = 0;
    int id = IdManager::next_id();
    long size_bytes = Rng::uniform(90, 300);
    // long density_cycles_bytes = Rng::uniform(200000, 230000);
    long density_cycles_bytes = Rng::normal(200000, 10000);
    double deadline = Rng::uniform(0.08, 0.1); // 80ms - 100ms
    bool offloaded = false;
    friend std::ostream& operator<<(std::ostream& out, const Task& t);
public:
    using PtrTask = std::shared_ptr<Task>;
    Task(const Simulator& sim) {
        timestamp = sim.now();
    }
    long total_cycles() const {
        return size_bytes * density_cycles_bytes;
    }
    int get_id() const {
        return id;
    }
    double get_deadline() const {
        return deadline;
    }
    double spent_time(Simulator& sim) {
        return sim.now() - timestamp;
    }
    bool get_offloaded() const {
        return offloaded;
    }
};

std::ostream& operator<<(std::ostream& out, const Task& t) {
    out << "Task { "
        << "timestamp=" << t.timestamp
        << ", id=" << t.id
        << ", size_bytes=" << t.size_bytes
        << ", density=" << t.density_cycles_bytes
        << ", total_cycles=" << t.total_cycles()
        << ", deadline=" << t.deadline
        << " }";
    return out;
}

enum class EventType {
    OnDecisionStart,
    OnDecisionComplete,
    OnProcessingStart,
    OnProcessingComplete,
};

class Model {
public:
    using PtrModel = std::shared_ptr<Model>;
    using Map = std::unordered_map<EventType, std::function<void(Simulator&)>>;
    virtual ~Model() = default;
    int get_id() const {
        return id;
    }
    void report_metric(Simulator& sim, const std::string& name, double value, const std::string& tag = "") {
        RECORD_METRIC(sim, this->get_id(), name, value, tag);
    }
    Map events;
protected:
    int id = IdManager::next_id();
};

enum class DecisionType {
    Local,
    Remote
};

class RSU;

struct DecisionResult {
    DecisionType decision_type = DecisionType::Local;
    Model::PtrModel choosed_device = nullptr;
};

enum class OffPolicyState {
    Idle,
    Busy
};

class OffPolicy {
    OffPolicyState current_state = OffPolicyState::Idle;
public:
    virtual DecisionResult decide(Task::PtrTask task) {
        return {DecisionType::Local, nullptr};
    }
    virtual double decision_time(Task::PtrTask task) {
        return Rng::uniform(0.003, 0.005); // 30–50 ms
    }
    void complete() {
        current_state = OffPolicyState::Idle;
    }
    void start() {
        current_state = OffPolicyState::Busy;
    }
    bool is_idle() const {
        return current_state == OffPolicyState::Idle;
    }
    bool is_busy() const {
        return current_state == OffPolicyState::Busy;
    }
};

enum class CPUState {
    Idle,
    Busy
};

class CPU {
    CPUState current_state = CPUState::Idle;
    double freq_mhz = 1.0 * 1e9;
public:
    virtual double processing_time(Task::PtrTask task) {
        return task->total_cycles() / freq_mhz;
    }
    void complete() {
        current_state = CPUState::Idle;
    }
    void start() {
        current_state = CPUState::Busy;
    }
    bool is_idle() const {
        return current_state == CPUState::Idle;
    }
    bool is_busy() const {
        return current_state == CPUState::Busy;
    }
    double get_freq() const {
        return freq_mhz;
    }
};

class DecisionEvent;
class CPUEvent;

class Vehicle: public Model, public std::enable_shared_from_this<Vehicle> {
    vec pos = {};
    vec vel = {};
    std::queue<Task::PtrTask> decision_queue;
    std::queue<Task::PtrTask> processing_queue;
    std::vector<Model::PtrModel> rsus;
    Task::PtrTask decision_task = nullptr;
    Task::PtrTask processing_task = nullptr;
public:
    OffPolicy off_policy;
    CPU cpu;
    using PtrVehicle = std::shared_ptr<Vehicle>;
    void onDecisionStart(Simulator& sim) {
        if (off_policy.is_idle() && !decision_queue.empty()) {
            // --- MÉTRICA: Tamanho da Fila ---
            // Isso permite plotar o "Queue backlog" ao longo do tempo
            report_metric(sim, "QueueSize_Decision", (double)decision_queue.size());
            decision_task = decision_queue.front();
            decision_queue.pop();
            off_policy.start();
            std::stringstream ss;
            ss << "Task " << decision_task->get_id()
               << " | Vehicle " << this->get_id()
               << " | DECISION_START"
               << " | queue_size=" << decision_queue.size();
            LOG_INFO(sim.now(), ss.str());
            sim.schedule<DecisionEvent>(
                sim.now() + off_policy.decision_time(decision_task),
                shared_from_this(),
                EventType::OnDecisionComplete
            );
        }
    }
    void onDecisionComplete(Simulator& sim) {
        std::stringstream ss;
        ss << "Task " << decision_task->get_id()
           << " | Vehicle " << this->get_id()
           << " | DECISION_COMPLETE";
        auto result = off_policy.decide(decision_task);
        ss << " | decision="
           << (result.decision_type == DecisionType::Local ? "Local" : "Remote");
        LOG_INFO(sim.now(), ss.str());
        off_policy.complete();
        if (result.decision_type == DecisionType::Local) {
            processing_queue.push(decision_task);
            if (cpu.is_idle()) {
                sim.schedule<CPUEvent>(
                    sim.now(),
                    shared_from_this(),
                    EventType::OnProcessingStart
                );
            }
        }
        decision_task = nullptr;
        if (!decision_queue.empty()) {
            sim.schedule<DecisionEvent>(
                sim.now(),
                shared_from_this(),
                EventType::OnDecisionStart
            );
        }
    }
    void OnProcessingStart(Simulator& sim) {
        if (cpu.is_idle() && !processing_queue.empty()) {
            processing_task = processing_queue.front();
            processing_queue.pop();
            std::stringstream ss;
            ss << "Task " << processing_task->get_id()
               << " | Vehicle " << this->get_id()
               << " | PROCESSING_START"
               << " | queue_size=" << processing_queue.size();
            LOG_INFO(sim.now(), ss.str());
            cpu.start();
            sim.schedule<CPUEvent>(
                sim.now() + cpu.processing_time(processing_task),
                shared_from_this(),
                EventType::OnProcessingComplete
            );
        }
    }
    void OnProcessingComplete(Simulator& sim) {
        std::stringstream ss;
        ss << "Task " << processing_task->get_id()
           << " | Vehicle " << this->get_id()
           << " | PROCESSING_COMPLETE"
           << " | completion_time=" << processing_task->spent_time(sim)
           << " | offloaded=" << (processing_task->get_offloaded() ? "Yes" : "No")
           << " | success=" << (processing_task->spent_time(sim) < processing_task->get_deadline() ? "Yes" : "No");
        LOG_INFO(sim.now(), ss.str());
        cpu.complete();
        // 1. Latência (Tempo Total no Sistema)
        double latency = processing_task->spent_time(sim);
        report_metric(sim, "TaskLatency", latency);

        // 2. Deadline Miss Ratio (0 ou 1)
        bool success = (latency <= processing_task->get_deadline());
        report_metric(sim, "TaskSuccess", success ? 1.0 : 0.0);

        // 3. Offloading Usage
        bool was_offloaded = processing_task->get_offloaded();
        report_metric(sim, "OffloadingType", was_offloaded ? 1.0 : 0.0, was_offloaded ? "Remote" : "Local");

        // 4. Energy Consumption (Modelo Simplificado: k * f^2 * t)
        // Suponha k=1e-28 (constante de capacitância efetiva)
        double energy = 1e-28 * std::pow(cpu.get_freq(), 2) * processing_task->total_cycles();
        report_metric(sim, "EnergyConsumption", energy);

        processing_task = nullptr;
        if (!processing_queue.empty()) {
            sim.schedule<CPUEvent>(
                sim.now() + micro_step,
                shared_from_this(),
                EventType::OnProcessingStart
            );
        }
    }
    Vehicle() {
        events[EventType::OnDecisionStart] = [this](Simulator& sim){
            Vehicle::onDecisionStart(sim);
        };
        events[EventType::OnDecisionComplete] = [this](Simulator& sim){
            Vehicle::onDecisionComplete(sim);
        };
        events[EventType::OnProcessingStart] = [this](Simulator& sim) {
            Vehicle::OnProcessingStart(sim);
        };
        events[EventType::OnProcessingComplete] = [this](Simulator& sim) {
            Vehicle::OnProcessingComplete(sim);
        };
    }
    void push(Task::PtrTask task) {
        decision_queue.push(task);
    }
};

class RSU: public Model {
    vec pos = {};
};

class DecisionEvent: public Event {
    Vehicle::PtrVehicle model = nullptr;
    EventType type;
public:
    DecisionEvent(double t, Vehicle::PtrVehicle model_, EventType type_):
        Event(t), model(model_), type(type_) {}
    void execute(Simulator& sim) override {
        model->events[type](sim);
    }
};

class CPUEvent: public Event {
    Model::PtrModel model = nullptr;
    EventType type;
public:
    CPUEvent(double t, Model::PtrModel model_, EventType type_):
        Event(t), model(model_), type(type_) {}
    void execute(Simulator& sim) override {
        model->events[type](sim);
    }
};

class TaskGenerationEvent: public Event {
    Vehicle::PtrVehicle model = nullptr;
    double lambda = 0.0;
public:
    TaskGenerationEvent(double t, Vehicle::PtrVehicle model_, double lambda_ = 2.0):
        Event(t), model(model_), lambda(lambda_) {
    }
    void execute(Simulator& sim) override {
        Task::PtrTask task = std::make_shared<Task>(sim);
        model->report_metric(sim, "TaskTotalCycles", task->total_cycles());
        std::stringstream ss;
        ss << "Task " << task->get_id()
           << " | Vehicle " << model->get_id()
           << " | TASK_GENERATED"
           << " | deadline=" << task->get_deadline();
        LOG_INFO(sim.now(), ss.str());
        model->push(task);
        sim.schedule<DecisionEvent>(sim.now() + micro_step, model, EventType::OnDecisionStart);
        double next_gen_event = Rng::exponential(lambda);
        sim.schedule<TaskGenerationEvent>(sim.now() + next_gen_event, model, lambda);
    }
};

int main() {
    auto csvCollector = std::make_shared<CSVMetricsCollector>("results/experiment_001.csv");
    MetricsHub::instance().addListener(csvCollector);
    std::vector<Vehicle::PtrVehicle> vehicles = {
        std::make_shared<Vehicle>(),
        // std::make_shared<Vehicle>(),
        // std::make_shared<Vehicle>()
    };
    Simulator sim;
    for (auto v: vehicles) {
        sim.schedule<TaskGenerationEvent>(Rng::uniform(0.0, 0.01), v, 1.0/Rng::uniform(0.02, 0.1));
    }
    sim.run(1000);
    return 0;
}
