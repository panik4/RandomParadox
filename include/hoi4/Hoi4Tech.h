#pragma once
#include "utils/SerialisationFwd.h"
#include <array>
#include <map>
#include <string>
#include <vector>
namespace Rpx::Hoi4 {


enum class TechEra { Interwar, Buildup, Early };

enum class NavalHullType { Light, Cruiser, Heavy, Carrier, Submarine };

struct Technology {
  std::string name;
  std::string predecessor;
  TechEra era;

  template<class Archive>
  void serialize(Archive &ar, const unsigned int /*version*/) {
    ar &name &predecessor &era;
  }
};


bool hasTechnology(const std::map<TechEra, std::vector<Technology>> &techs,
                   const std::string &techName);

} // namespace Rpx::Hoi4