#pragma once
#include "entities/Continent.h"
namespace Scenario {
class PdoxContinent : public FastWorldGen::Continent {
public:
  PdoxContinent(const Continent &continent);
  ~PdoxContinent();
};
} // namespace Scenario