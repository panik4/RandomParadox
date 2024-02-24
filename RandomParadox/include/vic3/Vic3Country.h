#pragma once
#include "generic/Country.h"
#include "vic3/Vic3Region.h"
#include "vic3/Vic3Utils.h"
#include <array>
#include <string>
#include <vector>

namespace Scenario::Vic3 {

class Country : public Scenario::Country {
  struct Market {
    std::vector<Country> marketParticipants;
  };

public:
  Country(std::string tag, int ID, std::string name, std::string adjective,
          Gfx::Flag flag);
  ~Country();
  std::vector<std::shared_ptr<Region>> ownedVic3Regions;
  std::vector<DiplomaticRelation> relations;
  std::map<std::string, double> summedPopNeeds;
  std::vector<Building> buildings;
  std::map<Good, double> producedGoods;



  std::vector<std::shared_ptr<Region>>
  getEligibleRegions(const std::string &resourceName);


};
} // namespace Scenario::Vic3