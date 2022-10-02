#pragma once
#include <string>
#include <vector>
namespace Scenario::Utils {
struct Coordinate {
  int x, z;
  double y, rotation;
};

struct Building {
  std::string name;
  Coordinate position;
  // sometimes necessary for special building types
  int relativeID;
};

struct UnitStack {
  int type;
  Coordinate position;
};

struct WeatherPosition {
  std::string effectSize;
  Coordinate position;
};
static Coordinate strToPos(const std::vector<std::string> &tokens,
                         const std::vector<int> positions) {
  Coordinate p;
  p.x = std::stoi(tokens[positions[0]]);
  p.y = std::stoi(tokens[positions[1]]);
  p.z = std::stoi(tokens[positions[2]]);
  p.rotation = std::stoi(tokens[positions[3]]);
  return p;
}
}; // namespace Scenario::Utils
