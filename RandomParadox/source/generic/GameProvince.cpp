#include "generic/GameProvince.h"
namespace Scenario {
GameProvince::GameProvince(Fwg::Province *province)
    : ID(province->ID), baseProvince(province), popFactor(0.0), devFactor(0.0),
      cityShare(0.0) {}

GameProvince::GameProvince()
    : ID(-1), baseProvince(nullptr), popFactor(0.0), devFactor(0.0),
      cityShare(0.0) {}

GameProvince::~GameProvince() {}
std::string GameProvince::toHexString() {
  std::string hexString = "x";
  std::stringstream stream;
  for (int i=2; i >=0; i--)
    stream << std::setfill('0') << std::setw(sizeof(char) * 2) << std::uppercase << std::hex
           << (int)this->baseProvince->colour.getBGR()[i];
  hexString.append(stream.str());
  return hexString;
}
} // namespace Scenario