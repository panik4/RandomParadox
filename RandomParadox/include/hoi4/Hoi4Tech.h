#pragma once
#include <array>
#include <string>
#include <vector>

namespace Scenario::Hoi4 {

enum class TechEra { Interwar, Buildup, Early };

enum class NavalHullType { Light, Cruiser, Heavy, Carrier, Submarine };

struct Technology {
  std::string name;
  std::string predecessor;
  TechEra era;
};
} // namespace Scenario::Hoi4