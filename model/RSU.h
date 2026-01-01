#ifndef RSU_H
#define RSU_H

#include <armadillo>
#include <memory>

#include "Model.h"

class RSU : public Model {
  arma::vec pos = {};

 public:
  using PtrRSU = std::shared_ptr<RSU>;
  RSU() : Model() { cpu.set_freq(Rng::uniform(4e9, 5e9)); }
  // RSU receives tasks via accept_processing_task (from Model)
};

#endif  // RSU_H
