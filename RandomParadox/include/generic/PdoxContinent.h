#pragma once
#include "entities/Continent.h"
class PdoxContinent : public Continent {
public:
  PdoxContinent(const Continent& continent);
  ~PdoxContinent();
};
