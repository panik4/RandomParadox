#pragma once
#include "FastWorldGenerator.h"
#include <string>
#include <vector>
namespace Scenario::Utils {

struct NoiseConfig {
  double fractalFrequency;
  double tanFactor;
  double cutOff;
  double mountainBonus;
};
static NoiseConfig defaultNoise{0.02, 0.0, 0.8, 2.0};
static NoiseConfig semiRareNoise{0.015, 0.0, 0.85, 2.0};
static NoiseConfig rareLargePatch{0.005, 0.0, 0.7, 0.0};
static NoiseConfig rareNoise{0.01, 0.0, 0.9, 2.0};
static NoiseConfig agriNoise{0.24, 0.0, 0.0, 0.0};

struct ResConfig {
  std::string name;
  bool capped;
  double resourcePrevalence;
  bool random = false;
  NoiseConfig noiseConfig;
  bool considerClimate = false;
  std::map<Fwg::Climate::Detail::ClimateTypeIndex, double>
      climateEffects;
  bool considerTrees = false;
  std::map<Fwg::Climate::Detail::TreeTypeIndex, double> treeEffects;
  bool considerSea = false;
  double oceanFactor = 0.0;
  double lakeFactor = 0.0;
};

struct Resource {
  std::string name;
  bool capped;
  double amount;
};

struct Coordinate {
  int x, z;
  double y, rotation;
};

struct Building {
  std::string name;
  Coordinate position;
  // sometimes necessary for special building types
  int relativeID;
  int provinceID;
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
struct Pathcfg {
  std::string modName;
  std::string gamePath;
  std::string gameModPath;
  std::string gameModsDirectory;
  std::string mappingPath;
  std::string resourcePath;
};

}; // namespace Scenario::Utils
