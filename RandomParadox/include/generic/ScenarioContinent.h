#pragma once
#include "entities/Continent.h"
namespace Scenario {
class ScenarioContinent : public Fwg::Areas::Continent {
public:
  ScenarioContinent(const Continent &continent);
  ~ScenarioContinent();
};
} // namespace Scenario