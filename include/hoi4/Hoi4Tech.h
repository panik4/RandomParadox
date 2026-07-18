#pragma once
#include "utils/Archive.h"
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

  void serialise(Fwg::Utils::Serialisation::Archive &ar) {
    ar &name &predecessor;
    ar.serialiseEnum(era);
  }
  void deserialise(Fwg::Utils::Serialisation::Archive &ar) { serialise(ar); }
};


bool hasTechnology(const std::map<TechEra, std::vector<Technology>> &techs,
                   const std::string &techName);

} // namespace Rpx::Hoi4