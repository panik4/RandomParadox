#include "vic3/Vic3Utils.h"

namespace Rpx::Vic3 {
std::vector<double> shiftedGaussian(double input) {
  std::vector<double> values(15, 0.0);

  // Calculate the mean and standard deviation based on the input
  double mean = 1.0 + input * 14.0;  // Scale mean between 1 and 15
  double stdDev = 0.1 + input * 5.0; // Scale std deviation between 0.1 and 2.1

  // Create a random number generator
  std::random_device rd;
  std::mt19937 gen(rd());
  std::normal_distribution<double> distribution(mean, stdDev);

  // Generate 15 values representing the distribution at each integer value
  // between 1 and 15
  for (int i = 0; i < 15; ++i) {
    double x = i + 1; // Value between 1 and 15
    values[i] = std::exp(-0.5 * std::pow((x - mean) / stdDev, 2)) /
                (stdDev * std::sqrt(2.0 * 3.14));
  }

  // Normalize the values such that their sum equals 1.0
  double sum = 0.0;
  for (double value : values) {
    sum += value;
  }
  for (double &value : values) {
    value /= sum;
  }

  return values;
}
} // namespace Rpx::Vic3