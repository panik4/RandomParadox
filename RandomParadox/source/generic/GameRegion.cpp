#include "generic/GameRegion.h"
namespace Scenario {
GameRegion::GameRegion() {}

GameRegion::GameRegion(const Region &baseRegion)
    : Region(baseRegion), assigned(false) {}

GameRegion::~GameRegion() {}
} // namespace Scenario