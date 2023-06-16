#pragma once
#include "entities/Continent.h"
namespace Scenario {
class ScenarioContinent : public Fwg::Continent {
public:
  ScenarioContinent(const Continent &continent);
  ~ScenarioContinent();
};
} // namespace Scenario