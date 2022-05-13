#include "generic/GameRegion.h"

GameRegion::GameRegion() {}

GameRegion::GameRegion(const Region &baseRegion)
    : Region(baseRegion), assigned(false) {
}

GameRegion::~GameRegion() {}
