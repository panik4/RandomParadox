#pragma once
#include "entities/Continent.h"
namespace Scenario {
class PdoxContinent : public Continent {
public:
  PdoxContinent(const Continent &continent);
  ~PdoxContinent();
};
} // namespace Scenario