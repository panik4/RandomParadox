#include "generic/GameProvince.h"
namespace Scenario {
GameProvince::GameProvince(Fwg::Province *province)
    : ID(province->ID), baseProvince(province), popFactor(0.0), devFactor(0.0),
      cityShare(0.0) {}

GameProvince::GameProvince()
    : ID(-1), baseProvince(nullptr), popFactor(0.0), devFactor(0.0),
      cityShare(0.0) {}

GameProvince::~GameProvince() {}
} // namespace Scenario