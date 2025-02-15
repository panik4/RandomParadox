#pragma once
#include "hoi4/Hoi4Country.h"

namespace Scenario::Hoi4::Selectors {
std::shared_ptr<Scenario::Hoi4::Region>
getRandomRegion(const std::shared_ptr<Scenario::Hoi4::Hoi4Country> &country);


std::shared_ptr<Scenario::Hoi4::Region>
getRandomCoastalRegion(const std::shared_ptr<Scenario::Hoi4::Hoi4Country> &country);






std::shared_ptr<Scenario::Hoi4::Hoi4Country> getOpposingIdeologyNeighbour(
    const Hoi4Country &country,
    const std::vector<std::shared_ptr<Scenario::Hoi4::Hoi4Country>>
        &hoi4Countries);

// method to get ALL opposing ideology neighbours
std::vector<std::shared_ptr<Scenario::Hoi4::Hoi4Country>>
getOpposingIdeologyNeighbours(
    const std::shared_ptr<Scenario::Hoi4::Hoi4Country> country,
    const std::vector<std::shared_ptr<Scenario::Hoi4::Hoi4Country>>
        &hoi4Countries);
// method to get ALL opposing ideology neighbours, that are neighbours of a country we already target
std::vector<std::shared_ptr<Scenario::Hoi4::Hoi4Country>>
getOpposingIdeologyNeighboursNeighbours(
    const std::shared_ptr<Scenario::Hoi4::Hoi4Country> country,
    const std::vector<std::shared_ptr<Scenario::Hoi4::Hoi4Country>>
        &hoi4Countries);

} // namespace Scenario::Hoi4::Selectors