#pragma once

#include <string>
#include <vector>
namespace Scenario {
enum class Gender { Male, Female };

enum class Ideology {
  Neutral,
  Fascist,
  Communist,
  Democratic,
  Monarchist,
  Anarchist,
  None
};
enum class Type {
  Leader,
  ArmyChief,
  NavyChief,
  AirForceChief,
  HighCommand,
  ArmyGeneral,
  FleetAdmiral,
  Politician,
  Theorist
};
class Character {
public:
  Character();
  ~Character();

  std::string name;
  std::string surname;
  Gender gender;

  Ideology ideology;
  Type type;
  std::vector<std::string> traits;
};
} // namespace Scenario