#include "hoi4/Hoi4ProvGen.h"

namespace Rpx::Hoi4 {

Arda::ScenarioPosition createPosition(Fwg::Position &position,
                                      Arda::PositionType type, int typeIndex,
                                      const std::vector<float> &altitudes) {
  Arda::ScenarioPosition scenarioPos;
  scenarioPos.position = position;
  scenarioPos.position.altitude =
      altitudes[scenarioPos.position.weightedCenter];
  scenarioPos.type = type;
  scenarioPos.typeIndex = typeIndex;
  return scenarioPos;
}
std::vector<Fwg::Areas::NeighbourProvince>
getNeighbourRelations(const std::shared_ptr<Fwg::Areas::Province> &prov,
                      const Fwg::Cfg &cfg, const float &factor) {
  // create the neighbour relations for each of the neighbours
  std::vector<Fwg::Areas::NeighbourProvince> neighbourRelations;
  for (auto &neighbour : prov->provinceNeighbours) {
    Fwg::Areas::NeighbourProvince neighbourProv;
    neighbourProv.neighbour = neighbour;
    neighbourProv.cost = 1.0f;
    double angle = 0.0;
    auto positionBetweenProvinces = prov->getPositionToNeighbourProvince(
        neighbour, cfg.width, angle, factor);
    neighbourProv.positionToNeighbour = positionBetweenProvinces;

    neighbourRelations.push_back(neighbourProv);
  }
  return neighbourRelations;
}

void generatePositions(Fwg::Terrain::TerrainData &terrainData,
                       const std::vector<std::shared_ptr<Arda::ArdaProvince>> &ardaProvinces) {
  const auto &altitudes = terrainData.altitudes;
  const auto &cfg = Fwg::Cfg::Values();
  for (auto &gameProv : ardaProvinces) {
    Fwg::Position position;
    if (gameProv->victoryPoint) {
      position = gameProv->victoryPoint->position;
    } else {
      position = gameProv->position;
    }

    gameProv->positions.push_back(createPosition(
        position, Arda::PositionType::VictoryPoint, 38, altitudes));

    // now we get neighbour relations for a province, but in a very short
    // distance to the centre.
    std::vector<Fwg::Areas::NeighbourProvince> neighbourRelations;
    const auto &prov = gameProv;
    neighbourRelations = getNeighbourRelations(prov, cfg, 0.2f);

    //  We use those for standstill, standstill RG, defending, attacking
    auto allowedSize = neighbourRelations.size() - 1;
    gameProv->positions.push_back(createPosition(
        neighbourRelations[std::min<int>(0, allowedSize)].positionToNeighbour,
        Arda::PositionType::Standstill, 0, altitudes));
    gameProv->positions.push_back(createPosition(
        neighbourRelations[std::min<int>(1, allowedSize)].positionToNeighbour,
        Arda::PositionType::StandstillRG, 21, altitudes));
    gameProv->positions.push_back(createPosition(
        neighbourRelations[std::min<int>(2, allowedSize)].positionToNeighbour,
        Arda::PositionType::Defending, 10, altitudes));
    gameProv->positions.push_back(createPosition(
        neighbourRelations[std::min<int>(3, allowedSize)].positionToNeighbour,
        Arda::PositionType::Attacking, 9, altitudes));

    // for sea we need: standstill, standstill RG, defending, attacking, per
    // neighbour: moving, disembarck (11 + x), moving RG, disembarck RG (30 +
    // x) for land we need: standstill, standstill RG, defending, attacking,
    // per neighbour: moving, moving RG for coastal land we need additionally:
    // ship in port (19), ship in port moving (20) commonality for all:
    // standstill (0), standstill RG (21), defending (10), attacking (9), per
    // neighbour: moving (1 + x), moving RG (22 + x), victory point (38)
    auto sea = gameProv->isSea();

    // evaluate if we need ship in port positions
    if (gameProv->isLand() && gameProv->isCoastalToOcean()) {
      // now check if we have a port location
      auto &locations = gameProv->locations;
      // get the port if we have one
      auto portLocation =
          std::find_if(locations.begin(), locations.end(), [](const auto &loc) {
            return loc->type == Fwg::Civilization::LocationType::Port;
          });
      if (portLocation == locations.end()) {
        // no port location, so we take random coastal pixels from the
        // baseProvince
        position = Fwg::Position(
            Fwg::Utils::Random::selectRandom(gameProv->coastalPixels),
            cfg.width);
      } else {
        position = (*portLocation)->position;
      }

      gameProv->positions.push_back(createPosition(
          position, Arda::PositionType::ShipInPort, 19, altitudes));
      gameProv->positions.push_back(createPosition(
          position, Arda::PositionType::ShipInPortMoving, 20, altitudes));
    }
    neighbourRelations = getNeighbourRelations(prov, cfg, 0.33f);
    // really close to the destination coast
    auto embarkNeighbourRelations = getNeighbourRelations(prov, cfg, 0.9f);
    auto embarkRgNeighbourRelations = getNeighbourRelations(prov, cfg, 0.8f);

    // all need moving, and moving RG. Moving we take from the
    // gameProv->neighbourRelations, while moving RG we take
    // from neighbourRelations, as they are closer to the center of the
    // province
    for (auto counter = 0; auto &neighbour : gameProv->neighbourRelations) {
      if (counter > 7) {
        // hoi4 only supports 8 moving positions, so we break here
        break;
      }

      gameProv->positions.push_back(createPosition(
          neighbour->positionToNeighbour, Arda::PositionType::UnitMoving,
          1 + counter, altitudes));
      gameProv->positions.push_back(createPosition(
          neighbourRelations[counter].positionToNeighbour,
          Arda::PositionType::UnitMovingRG, 22 + counter, altitudes));
      if (sea) {
        // now we add the embark positions, which are the same as the
        // neighbour relations, but with a different type
        gameProv->positions.push_back(createPosition(
            embarkNeighbourRelations[counter].positionToNeighbour,
            Arda::PositionType::UnitDisembarking, 11 + counter, altitudes));

        gameProv->positions.push_back(createPosition(
            embarkRgNeighbourRelations[counter].positionToNeighbour,
            Arda::PositionType::UnitDisembarkingRG, 30 + counter, altitudes));
        counter++;
      }
    }
    // now sort by typeIndex
    std::sort(
        gameProv->positions.begin(), gameProv->positions.end(),
        [](const Arda::ScenarioPosition &a, const Arda::ScenarioPosition &b) {
          return a.typeIndex < b.typeIndex;
        });
  }
}

} // namespace Rpx::Hoi4