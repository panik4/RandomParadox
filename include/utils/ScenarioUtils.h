#pragma once
#include "FastWorldGenerator.h"
#include <string>
#include <vector>
enum class GameType { Generic, Hoi4, Vic3, Eu4 };
namespace Scenario::Utils {
struct Pathcfg {
  std::string modName;
  std::string gamePath;
  std::string gameModPath;
  std::string gameModsDirectory;
  std::string mappingPath;
  std::string resourcePath;
};

}; // namespace Scenario::Utils
