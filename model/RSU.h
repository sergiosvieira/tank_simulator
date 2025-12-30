#ifndef RSU_H
#define RSU_H

#include "Model.h"
#include <armadillo>
#include <memory>

class RSU : public Model {
  arma::vec pos = {};

public:
  using PtrRSU = std::shared_ptr<RSU>;
  // RSU receives tasks via accept_processing_task (from Model)
};

#endif // RSU_H
