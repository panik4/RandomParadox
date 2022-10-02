#pragma once
#include <string>
namespace Scenario::Utils {
struct Position {
  int x, z;
  double y, rotation;
};

struct Building {
  std::string name;
  Position position;
  // sometimes necessary for special building types
  int relativeID;
};

struct UnitStack {
  int type;
  Position position;
};

}; // namespace Scenario::Utils
