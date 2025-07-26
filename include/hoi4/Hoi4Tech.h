#pragma once
#include <array>
#include <string>
#include <vector>
#include <map>
namespace Rpx::Hoi4 {


enum class TechEra { Interwar, Buildup, Early };

enum class NavalHullType { Light, Cruiser, Heavy, Carrier, Submarine };

struct Technology {
  std::string name;
  std::string predecessor;
  TechEra era;
};


bool hasTechnology(const std::map<TechEra, std::vector<Technology>> &techs,
                   const std::string &techName);

} // namespace Rpx::Hoi4