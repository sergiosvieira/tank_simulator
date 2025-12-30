#ifndef RNG_H
#define RNG_H

#include <random>
#include <stdexcept>

struct Rng {
  static constexpr int seed = 1978;
  static std::mt19937 &engine() {
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

  template <typename T>
  static T sample(const std::vector<T>& vec) {
      if (vec.empty()) {
          throw std::runtime_error("Rng::sample: Attempted to sample from an empty vector.");
      }
      // Usa size_t para garantir compatibilidade com o tamanho máximo do vetor
      std::uniform_int_distribution<size_t> d(0, vec.size() - 1);
      return vec[d(engine())];
  }
};

#endif // RNG_H
