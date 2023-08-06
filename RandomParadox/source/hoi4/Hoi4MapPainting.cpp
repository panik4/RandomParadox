#include "hoi4/Hoi4MapPainting.h"

namespace Scenario::Hoi4::MapPainting {

namespace Countries {
namespace Detail {
void trackChanges(Generator &hoi4Gen, const Fwg::Gfx::Bitmap readInCountryMap,
                  ChangeHolder &changes) {
  Fwg::Utils::Logging::logLine("Tracking changes to countries");
  for (auto &country : hoi4Gen.hoi4Countries) {
    country.second.hoi4Regions.clear();
    country.second.ownedRegions.clear();
  }
  std::map<int, std::string> ownership;
  // for every already existing state
  for (auto &state : hoi4Gen.gameRegions) {
    // save previous owner
    std::string previousOwnerCountry = state->owner;
    std::map<std::string, int> potentialOwners;
    // iterate over currently owned provinces
    for (const auto &province : state->gameProvinces) {
      // and their pixels
      for (const auto pixel : province->baseProvince->pixels) {
        // and determine who owns it in the edited image
        const auto &col = readInCountryMap[pixel];
        if (hoi4Gen.colourMap.find(col)) {
          potentialOwners[hoi4Gen.colourMap[col]]++;
        }
      }
    }
    // now determine new owner
    using pair_type = decltype(potentialOwners)::value_type;
    auto likeliestOwner =
        std::max_element(std::begin(potentialOwners), std::end(potentialOwners),
                         [](const pair_type &p1, const pair_type &p2) {
                           return p1.second < p2.second;
                         });
    auto &newOwnerCountry = hoi4Gen.hoi4Countries.at(likeliestOwner->first);
    state->owner = newOwnerCountry.tag;
    // track the change if the owner changed
    if (newOwnerCountry.tag != previousOwnerCountry) {
      // give the new owner a region
      newOwnerCountry.ownedRegions.push_back(state->ID);
      auto &prevCountry = hoi4Gen.hoi4Countries.at(previousOwnerCountry);
      for (auto st = 0; st < prevCountry.ownedRegions.size(); st++) {
        if (prevCountry.ownedRegions[st] == state->ID) {
          prevCountry.ownedRegions.erase(prevCountry.ownedRegions.begin() + st);
          st--;
          Fwg::Utils::Logging::logLine("Deleting state ", state->ID,
                                       " from country: ", previousOwnerCountry);
        }
      }

      Fwg::Utils::Logging::logLine("State ", state->ID, " changed owner from ",
                                   previousOwnerCountry, " to ",
                                   newOwnerCountry.tag);
      changes.ownerChanges.insert(
          {state, {previousOwnerCountry, newOwnerCountry.tag}});
    }
  }
  // track the change if the state was deleted
  for (auto &country : hoi4Gen.hoi4Countries) {
    if (country.second.hoi4Regions.size() == 0) {
      changes.deletedCountries.insert(country.second.tag);
    }
  }
}

Fwg::Gfx::Bitmap createCountryBitmap(const Generator &hoi4Gen,
                                     Fwg::Gfx::Bitmap &provinceMap) {
  Fwg::Utils::Logging::logLine("Creating Country Map from game files");
  Fwg::Gfx::Bitmap countries(provinceMap.bInfoHeader.biWidth,
                             provinceMap.bInfoHeader.biHeight, 24);
  std::set<int> stateBorders;
  // for every country
  for (const auto &country : hoi4Gen.hoi4Countries) {
    // and its states
    for (const auto &state : country.second.hoi4Regions) {
      std::set<int> statePixels;
      auto col = country.second.colour;
      // check all provinces and paint the map at their locations
      for (const auto &prov : state->gameProvinces) {
        for (const auto pixel : prov->baseProvince->pixels) {
          if (pixel >= 0 && pixel < provinceMap.size()) {
            statePixels.insert(pixel);
            countries.imageData[pixel] = col;
          }
        }
      }
      // now save the borders of states
      for (const auto pixel : statePixels) {
        std::array<int, 4> newPixels = {pixel + 1, pixel - 1,
                                        pixel + countries.bInfoHeader.biWidth,
                                        pixel - countries.bInfoHeader.biWidth};
        for (const auto newPix : newPixels) {
          if (statePixels.find(newPix) == statePixels.end()) {
            stateBorders.insert(pixel);
          }
        }
      }
    }
  }
  // and write the borders of states into the image
  for (const auto borderPix : stateBorders) {
    countries.imageData[borderPix] = Fwg::Gfx::Colour{254, 253, 255};
  }
  return countries;
}

} // namespace Detail
void updateCountries(Generator &hoi4Gen, ChangeHolder &changes) {
  for (auto &countryEntry : hoi4Gen.hoi4Countries) {
    auto &country = countryEntry.second;
    for (auto stID = 0; stID < country.hoi4Regions.size(); stID++) {
      if (changes.deletedStates.find(country.hoi4Regions[stID]->ID) !=
          changes.deletedStates.end()) {
        // the state was deleted, so remove it from here as well
      }
    }
  }
}

void edit(const std::string &inPath, const std::string &outputPath,
          const std::string &mapName, Generator &hoi4Gen,
          Fwg::Gfx::Bitmap &provinceMap, ChangeHolder &changes) {
  Fwg::Utils::Logging::logLine("Editing Countries");
  auto countryMap = Detail::createCountryBitmap(hoi4Gen, provinceMap);

  auto &config = Fwg::Cfg::Values();
  // edit and save in editedMaps directory
  Fwg::Gfx::Bmp::edit<Fwg::Gfx::Colour>("countries.bmp", countryMap,
                                        "countryMap", config.mapsPath,
                                        config.mapsToEdit, config.editor);

  // we now have a changed country bitmap, which allows us to track the done
  // changes it makes sense to overlay the new map over the new state of
  // provinces and states

  // determine which states are now in different hands, and who should have
  // them, and cores...
  Detail::trackChanges(hoi4Gen, countryMap, changes);
}
} // namespace Countries

namespace States {
namespace Detail {
void trackChanges(Generator &hoi4Gen, const Fwg::Gfx::Bitmap readInStateMap,
                  ChangeHolder &changes) {
  Fwg::Utils::Logging::logLine("Tracking changes to states");
  for (auto &state : hoi4Gen.hoi4States) {
    state->gameProvinces.clear();
  }
  std::map<int, std::string> ownership;
  for (auto &gameProvince : hoi4Gen.gameProvinces) {
    if (gameProvince->baseProvince->sea ||
        gameProvince->baseProvince->regionID > hoi4Gen.gameRegions.size())
      continue;
    auto &prevOwnerState =
        hoi4Gen.gameRegions[gameProvince->baseProvince->regionID];
    std::map<int, int> potentialOwners;
    for (auto pixel : gameProvince->baseProvince->pixels) {
      // and determine which state owns it in the edited image
      const auto &col = readInStateMap[pixel];
      if (hoi4Gen.stateColours.find(col)) {
        auto owner = hoi4Gen.stateColours[col];
        potentialOwners[owner->ID]++;
      } else {
        // std::cout << "Create new state" << std::endl;
      }
    }
    // now determine new owner
    if (potentialOwners.size()) {
      using pair_type = decltype(potentialOwners)::value_type;
      auto likeliestOwner = std::max_element(
          std::begin(potentialOwners), std::end(potentialOwners),
          [](const pair_type &p1, const pair_type &p2) {
            return p1.second < p2.second;
          });
      auto &newOwnerState = hoi4Gen.gameRegions.at(likeliestOwner->first);

      // track the change if the owner changed
      if (newOwnerState != prevOwnerState) {
        // both involved states were changed
        changes.changedStates.insert(newOwnerState->ID);
        changes.changedStates.insert(prevOwnerState->ID);
        // give the new owner a region
        newOwnerState->gameProvinces.push_back(gameProvince);
        Fwg::Utils::Logging::logLine(
            "Province ", gameProvince->ID, " changed region from ",
            prevOwnerState->ID, " to ", newOwnerState->ID);
        changes.stateChanges.insert(
            {gameProvince, {prevOwnerState, newOwnerState}});
        for (auto pI = 0; pI < prevOwnerState->gameProvinces.size(); pI++) {
          if (prevOwnerState->gameProvinces[pI]->ID == gameProvince->ID) {
            prevOwnerState->gameProvinces.erase(
                prevOwnerState->gameProvinces.begin() + pI);
            pI--;
            std::cout << "State Track Changes: Deleting province from state: "
                      << prevOwnerState->ID << std::endl;
          }
        }
      }
    }
    // prevOwnerState->gameProvinces.push_back(gameProvince);
  }
  for (auto i = 0; i < hoi4Gen.gameRegions.size(); i++) {
    changes.stateIdMapping[i] = 0;
  }
  // track changes in IDs of regions
  for (auto i = 0; i < hoi4Gen.gameRegions.size(); i++) {
    if (!hoi4Gen.gameRegions[i]->gameProvinces.size()) {
      // remove this state from the country as well
      auto &c = hoi4Gen.countries.at(hoi4Gen.gameRegions[i]->owner);
      c.ownedRegions.erase(std::find(c.ownedRegions.begin(),
                                     c.ownedRegions.end(),
                                     hoi4Gen.gameRegions[i]->ID));
      // remove it from game regions as well
      hoi4Gen.gameRegions.erase(hoi4Gen.gameRegions.begin() + i);
      //  state was removed
      changes.deletedStates.insert(i);
      // every succeeding state has their ID modified by -1
      for (auto x = i + 1; x < changes.stateIdMapping.size(); x++)
        changes.stateIdMapping.at(x)--;
      i--;
    }
  }
  for (auto &gameProv : hoi4Gen.gameProvinces) {
    if (!gameProv->baseProvince->sea && !gameProv->baseProvince->isLake)
      gameProv->baseProvince->regionID +=
          changes.stateIdMapping.at(gameProv->baseProvince->regionID);
  }
  for (auto i = 0; i < hoi4Gen.gameRegions.size(); i++) {
    hoi4Gen.gameRegions[i]->ID = i;
  }
  // now update all countries by changing the IDs they own according to the
  // stateIdMapping
  for (auto &c : hoi4Gen.countries) {
    for (auto &state : c.second.ownedRegions) {
      state += changes.stateIdMapping.at(state);
    }
  }
  Fwg::Utils::Logging::logLine("Done tracking changes to states");
}

Fwg::Gfx::Bitmap createStateBitmap(const Generator &hoi4Gen,
                                   const Fwg::Gfx::Bitmap &provinceMap) {
  Fwg::Utils::Logging::logLine("Creating State Image from game files");
  Fwg::Gfx::Bitmap states(provinceMap.bInfoHeader.biWidth,
                          provinceMap.bInfoHeader.biHeight, 24);
  std::set<int> stateBorders;
  std::set<int> provinceBorders;

  for (const auto &state : hoi4Gen.hoi4States) {
    std::set<int> statePixels;
    for (const auto province : state->gameProvinces) {
      std::copy(province->baseProvince->pixels.begin(),
                province->baseProvince->pixels.end(),
                std::inserter(statePixels, statePixels.end()));
      std::copy(province->baseProvince->borderPixels.begin(),
                province->baseProvince->borderPixels.end(),
                std::inserter(provinceBorders, provinceBorders.end()));
    }
    for (const auto pixel : statePixels) {
      states.imageData[pixel] = state->colour;
      std::array<int, 4> newPixels = {pixel + 1, pixel - 1,
                                      pixel + states.bInfoHeader.biWidth,
                                      pixel - states.bInfoHeader.biWidth};
      for (const auto newPix : newPixels) {
        if (statePixels.find(newPix) == statePixels.end()) {
          stateBorders.insert(pixel);
        }
      }
    }
  }
  //for (const auto borderPix : stateBorders) {
  //  states.imageData[borderPix] = Fwg::Gfx::Colour{254, 253, 255};
  //}
  //for (const auto borderPix : provinceBorders) {
  //  states.imageData[borderPix] = Fwg::Gfx::Colour{128, 253, 255};
  //}
  return states;
}
} // namespace Detail

void updateStates(Generator &hoi4Gen, ChangeHolder &changes) {
  auto oldGPs = hoi4Gen.gameProvinces;
  auto oldGRs = hoi4Gen.gameRegions;

  auto &stateColours = hoi4Gen.stateColours;
  for (auto &newProv : changes.newProvs) {

    Fwg::Gfx::Colour colour;
    Scenario::Region r;
    auto reg = std::make_shared<Scenario::Region>(r);
    reg->ID = hoi4Gen.gameRegions.size();
    // pick a random, but unique colour
    do {
      colour.randomize();
    } while (stateColours.find(colour));
    reg->colour = colour;
    reg->gameProvinces.push_back(hoi4Gen.gameProvinces[newProv]);
    hoi4Gen.gameProvinces[newProv]->baseProvince->regionID = reg->ID;
    hoi4Gen.gameRegions.push_back(reg);
    hoi4Gen.stateColours.setValue(reg->colour, reg);
  }

  // now for every state, check if province changes apply
  for (auto st = 0; st < hoi4Gen.gameRegions.size(); st++) {
    auto &state = hoi4Gen.gameRegions[st];
    // for every province
    for (auto i = 0; i < state->gameProvinces.size(); i++) {
      if (changes.deletedProvs.find(state->gameProvinces[i]->ID) !=
          changes.deletedProvs.end()) {
        Fwg::Utils::Logging::logLine(
            "State::Update States: Deleted province from state ", state->ID,
            " with ID: ", state->gameProvinces[i]->ID);

        // remove province from state, as it was deleted
        state->gameProvinces.erase(state->gameProvinces.begin() + i);
        i--;
      }
    }
    // ID Mapping is only necessary if provinces were edited. The ID-mapping is
    // empty, if not
    if (changes.provIdMapping.size()) {
      for (auto i = 0; i < state->gameProvinces.size(); i++) {
        //// check if a province was changed
        //// therefore reset it to the read in one
        //// get the current ID of the province and check, what the new one must
        //// be
        auto oldID = state->gameProvinces[i]->ID;
        auto newId =
            state->gameProvinces[i]->ID + changes.provIdMapping.at(oldID);
        // then take overwrite the province we have with the read in one
        state->gameProvinces[i] = hoi4Gen.gameProvinces[newId];
        // that means, that region ID for old and new province should be the
        // same, as the state ownership has not changed
        /* if (state->gameProvinces[i]->baseProvince->regionID != state->ID) {
          std::cout << state->gameProvinces[i]->baseProvince->ID << std::endl;
          auto oldState = oldGRs[st];
          std::cout << "Old State Provinces " << std::endl;
          std::cout << "NewID: " << newId << std::endl;
          std::cout << "OldID: " << oldID << std::endl;
          std::cout << "Province Region ID: "
                    << state->gameProvinces[i]->baseProvince->regionID
                    << std::endl;
          std::cout << "State ID: " << state->ID << std::endl;
        }*/
      }
    }
  }

  bool stillUnassigned = false;
  do {
    stillUnassigned = false;
    for (auto newProvID : changes.newProvs) {
      newProvID -= changes.deletedProvs.size();
      auto &newProv = hoi4Gen.gameProvinces[newProvID];
      if (newProv->baseProvince->regionID == 1000000 &&
          !newProv->baseProvince->sea) {
        stillUnassigned = true;
        for (auto &neigbour : newProv->neighbours) {
          if (neigbour.baseProvince->regionID != 1000000) {
            hoi4Gen.gameRegions[neigbour.baseProvince->regionID]
                ->gameProvinces.push_back(newProv);
            newProv->baseProvince->regionID = neigbour.baseProvince->regionID;
          }
        }
      }
    }
  } while (stillUnassigned);
}

void edit(const std::string &inPath, const std::string &outputPath,
          const std::string &mapName, Generator &hoi4Gen,
          const Fwg::Gfx::Bitmap &provinceMap, ChangeHolder &changes) {
  Fwg::Utils::Logging::logLine("Editing States");
  auto &config = Fwg::Cfg::Values();
  auto stateMap = Detail::createStateBitmap(hoi4Gen, provinceMap);
  // edit and save in editedMaps directory
  Fwg::Gfx::Bmp::edit<Fwg::Gfx::Colour>("states.bmp", stateMap, "stateMap",
                                        config.mapsPath, config.mapsToEdit,
                                        config.editor);
  Detail::trackChanges(hoi4Gen, stateMap, changes);
}
} // namespace States

namespace Provinces {

namespace Detail {
void trackChanges(Generator &hoi4Gen, const Fwg::Gfx::Bitmap readInProvMap,
                  ChangeHolder &changes, Fwg::Gfx::Bitmap &heightMap,
                  Fwg::Areas::AreaData &areaNewData) {
  Fwg::Areas::Provinces::readProvinceBMP(readInProvMap, heightMap,
                                         areaNewData.provinces,
                                         areaNewData.provinceColourMap, true);

  for (auto &reg : areaNewData.regions)
    reg.provinces.clear();

  // now compare both maps and see which province was deleted
  for (auto i = 0; i < hoi4Gen.areas.provinces.size() + 1; i++) {
    changes.provIdMapping[i] = 0;
  }

  // now check for new provinces
  for (auto i = hoi4Gen.areas.provinces.size();
       i < areaNewData.provinces.size(); i++) {
    Fwg::Utils::Logging::logLine("Added new province with ID: ", i);
    changes.newProvs.insert(i);
  }
  // track changes in IDs
  for (auto i = 0; i < hoi4Gen.areas.provinces.size(); i++) {
    if (hoi4Gen.areas.provinces[i]->pixels.size() !=
        areaNewData.provinces[i]->pixels.size()) {
      //  we have SOME change and the province still exists
      changes.changedProvs.insert(i);
      // the province was removed
      if (!areaNewData.provinces[i]->pixels.size()) {
        changes.deletedProvs.insert(i);
        Fwg::Utils::Logging::logLine("Deleted province with ID: ", i);
        delete areaNewData.provinces[i];
        // set it to nullptr
        areaNewData.provinces[i] = nullptr;
        // every succeeding province has their ID modified by -1
        for (auto x = i + 1; x < hoi4Gen.areas.provinces.size(); x++)
          changes.provIdMapping.at(x)--;
      }
    }
  }
  // now set new IDs
  for (auto i = 0; i < areaNewData.provinces.size(); i++) {
    if (areaNewData.provinces[i] == nullptr) {
      areaNewData.provinces.erase(areaNewData.provinces.begin() + i);
      i--;
    } else {
      areaNewData.provinces[i]->ID = i;
    }
  }
  // overwrite areas
  hoi4Gen.areas = areaNewData;
}
std::vector<std::vector<std::string>> readDefinitions(const std::string &path) {
  auto list = Fwg::Parsing::getLinesByID(path);
  return list;
}

bool isProvinceID(std::string &content, const std::string &delimiterLeft,
                  const std::string &delimiterRight, const int startPos,
                  const std::map<int, int> &replacementRules,
                  std::set<int> deletedIDs) {
  auto leftDelim =
      delimiterLeft.size() ? content.find(delimiterLeft, startPos) : 0;
  // we dont want the delimiter inside!
  if (leftDelim != content.npos)
    leftDelim += delimiterLeft.size();

  auto rightDelim = content.find(delimiterRight, leftDelim);
  while (leftDelim != content.npos && rightDelim != content.npos) {
    bool replaced = false;
    auto substr = content.substr(leftDelim, rightDelim - leftDelim);

    std::string replacementString{""};
    auto tokens = Fwg::Parsing::getTokens(substr, ' ');
    for (auto &token : tokens) {
      if (Fwg::Utils::isInt(token)) {
        auto repVal = std::stoi(token);
        if (deletedIDs.find(repVal) == deletedIDs.end()) {
          // modification mode
          if (replacementRules.at(repVal) != 0) {
            repVal = repVal + replacementRules.at(repVal);
            replaced = true;
          }
          replacementString.append(std::to_string(repVal));
          replacementString.append(" ");
        } else {
          // deletion mode: delete reference to this province, by not adding it
          // to replacementString
        }
      } else {
        replacementString.append(token);
      }
    }
    // tokenize all contents between delimiters
    // should always be spaces afaik
    // auto provIDs = Parsing::Scenario::getNumbers(substr, ' ');
    // std::string replacementString{""};
    // for (auto &provID : provIDs) {
    //  auto repVal = provID;
    //  if (replacementRules.at(provID) != 0) {
    //    repVal = provID + replacementRules.at(provID);
    //    replaced = true;
    //  }
    //  replacementString.append(std::to_string(repVal));
    //  replacementString.append(" ");
    //}

    // do the replace
    content.erase(leftDelim, rightDelim - leftDelim);
    content.insert(leftDelim, replacementString);
    if (replaced)
      std::cout << "replaced" << substr << " with " << replacementString
                << std::endl;

    // for next search in while
    leftDelim = delimiterLeft.size() ? content.find(delimiterLeft, rightDelim)
                                     : std::string::npos;
    // we dont want the delimiter inside!
    if (leftDelim != content.npos)
      leftDelim += delimiterLeft.size();
    // fot next search in while
    rightDelim = content.find(delimiterRight, leftDelim);
  }
}

} // namespace Detail
void edit(const std::string &inPath, const std::string &outputPath,
          Fwg::Gfx::Bitmap &provMap, Generator &hoi4Gen,
          ChangeHolder &changes) {
  Fwg::Utils::Logging::logLine("Editing Provinces");
  auto heightMap = Fwg::Gfx::Bmp::load24Bit(
      inPath + "map//" + "heightmap" + ".bmp", "heightmap");
  using namespace Fwg::Areas;
  // now read in new file and compare data
  AreaData areaNewData;
  for (auto &prov : hoi4Gen.areas.provinces) {
    Fwg::Province *newProv = new Fwg::Province();
    *newProv = *prov;
    newProv->pixels.clear();
    newProv->coastalPixels.clear();
    newProv->borderPixels.clear();
    newProv->cityPixels.clear();
    areaNewData.provinceColourMap.setValue(newProv->colour, newProv);
    areaNewData.provinces.push_back(newProv);
  }

  auto &config = Fwg::Cfg::Values();
  Fwg::Gfx::Bmp::edit<Fwg::Gfx::Colour>("provinces.bmp", provMap, "provinceMap",
                                        config.mapsPath, config.mapsToEdit,
                                        config.editor);
  hoi4Gen.provinceMap = provMap;
  // save edited map into mod folder
  Fwg::Gfx::Bmp::save(provMap, outputPath + "map//provinces.bmp");
  Detail::trackChanges(hoi4Gen, provMap, changes, heightMap, areaNewData);

  // provinces are referenced: history: units, states
  // map: definition.csv, airports, adjacencies, railways, rocketsites, supply
  // nodes, unit_stacks need a rule set for every file type

  // for (const auto &edit : mapFilesToEdit) {
  //   std::string out{""};
  //   auto fileContent = Parsing::Scenario::getLines(inPath + edit);
  //   for (auto &line : fileContent) {
  //     isProvinceID(line, "{", "}", 0, IDTransformations, deletedIDs);
  //   }
  //   for (auto &line : fileContent) {
  //     out.append(line + "\n");
  //   }
  //   Parsing::Scenario::writeFile(outputPath + edit, out);
  // }
  // mapFilesToEdit = {std::string("//map//definition.csv")};
  // for (const auto &edit : mapFilesToEdit) {
  //  std::string out{""};
  //  auto fileContent = Parsing::Scenario::getLines(inPath + edit);
  //  for (auto &line : fileContent) {
  //    isProvinceID(line, "", ";", 0, IDTransformations, deletedIDs);
  //  }
  //  for (auto &line : fileContent) {
  //    out.append(line + "\n");
  //  }
  //  Parsing::Scenario::writeFile(outputPath + edit, out);
  //}

  // mapFilesToEdit = {std::string("//map//airports.txt"),
  //                   std::string("//map//rocketsites.txt")};
  // for (const auto &edit : mapFilesToEdit) {
  //   std::string out{""};
  //   auto fileContent = Parsing::Scenario::getLines(inPath + edit);
  //   for (auto &line : fileContent) {
  //     isProvinceID(line, "{", "}", 0, IDTransformations);
  //   }
  //   for (auto &line : fileContent) {
  //     out.append(line + "\n");
  //   }
  //   Parsing::Scenario::writeFile(outputPath + edit, out);
}
} // namespace Provinces

void runMapEditor(Generator &hoi4Gen, const std::string &mappingPath,
                  const std::string &gameModPath,
                  Scenario::Gfx::FormatConverter &formatConverter) {
  /* generate world from input heightmap
   * compare differences between heightmaps for edit mask
   *  merge all maps with mask, so rest stays the same
   * for province map, take notice which provinces are getting removed
   * replace province IDs by replaced provinces. Recalculate positions for these
   * provinces add these provinces to empty states
   *
   */
  Scenario::Hoi4::MapPainting::ChangeHolder changes;
  // only load the default map once!
  auto provinceMap =
      Fwg::Gfx::Bmp::load24Bit(mappingPath + "map//provinces.bmp", "provinces");
  // in case we want to edit provinces
  while (true) {
    Fwg::Utils::Logging::logLine("1) Edit Provinces, 2) Edit States, 3) Edit "
                                 "Countries, 4) Generate data");
    int choice;
    std::cin >> choice;
    // first edit province.bmp, and update some relevant files
    auto &config = Fwg::Cfg::Values();
    config.mapsToEdit.insert("provinceMap");
    config.mapsToEdit.insert("stateMap");
    config.mapsToEdit.insert("countryMap");
    config.width = provinceMap.bInfoHeader.biWidth;
    config.height = provinceMap.bInfoHeader.biHeight;

    switch (choice) {
    case 1: {
      Scenario::Hoi4::MapPainting::Provinces::edit(
          mappingPath, gameModPath, provinceMap, hoi4Gen, changes);
      // map them again
      hoi4Gen.mapProvinces();
      Scenario::Hoi4::MapPainting::States::updateStates(hoi4Gen, changes);
      break;
    }
    case 2: {
      hoi4Gen.initializeCountries();
      Scenario::Hoi4::MapPainting::States::edit(mappingPath, gameModPath,
                                                "states.bmp", hoi4Gen,
                                                provinceMap, changes);
      break;
    }
    case 3: {
      hoi4Gen.initializeCountries();
      Scenario::Hoi4::MapPainting::Countries::edit(mappingPath, gameModPath,
                                                   "countries.bmp", hoi4Gen,
                                                   provinceMap, changes);
      break;
    }
    case 4: {
      std::filesystem::copy_file(
          "resources//hoi4//map//climate.txt", mappingPath + "map//terrain.txt",
          std::filesystem::copy_options::update_existing);
      auto climateMap = Fwg::IO::Reader::readClimateImage(
          mappingPath + "map//terrain.bmp", config);
      // set terrain types of base fwg provinces
      Fwg::ClimateGeneration::provinceTerrainTypes(hoi4Gen.areas.provinces,
                                                   climateMap);
      // get the provinces into GameProvinces
      hoi4Gen.mapProvinces();
      // hoi4Gen.mapTerrain();
      Hoi4::Parsing::Writing::definition(mappingPath + "map//definition.csv",
                                         hoi4Gen.gameProvinces);
      break;
    }
    case 5: {
      int factor = 1;
      do {
        auto heightMap = Fwg::Gfx::Bmp::load24Bit(
            mappingPath + "map//heightmap.bmp", "heightmap");
        auto normalMap = Fwg::Gfx::Bmp::sobelFilter(heightMap, factor);
        formatConverter.dumpWorldNormal(
            normalMap, mappingPath + "map//world_normal.bmp", false);
        Fwg::Gfx::Images::show(mappingPath + "map//world_normal.bmp");
        Fwg::Utils::Logging::logLine(
            "Happy? You may increase the current factor of ", factor,
            " by typing in the desired factor now, or type 0 to stop and "
            "return to the previous menu");
        std::cin >> factor;
      } while (factor != 0);

      break;
    }
    }

    // finalize edits
    // get the new internal representation of the game state into mod files

    // update province related files
    Hoi4::Parsing::Writing::definition(gameModPath + "\\map\\definition.csv",
                                       hoi4Gen.gameProvinces);

    // update states according to previously generated (and potentially edited)
    // state map will automatically correct province assignment
    /* Must edit
     *   - airfields
     *   - rocketsites
     *   - buildings.txt
     *   - supply_nodes.txt
     *   - weatherpositions
     *
     *
     *
     */

    /* UPDATE references to province IDs
     *  Map files:
     *   - unitstacks.txt
     *   - definition.csv
     *   - strategic regions
     *
     * History files:
     *   -
     *
     *
     * Common files:
     *   - events
     *   - decisions
     *   - ...?
     *
     *
     *
     */
  }
}

} // namespace Scenario::Hoi4::MapPainting
