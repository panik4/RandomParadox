#include "GameRegion.h"

GameRegion::GameRegion() {}

GameRegion::GameRegion(const Region &baseRegion)
    : Region(baseRegion), assigned(false) {
  //this->ID = baseRegion.ID;
  //this->sea = baseRegion.sea;
}

GameRegion::~GameRegion() {}
