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
  auto ID = 0;
  // for every already existing state
  for (const auto &state : hoi4Gen.hoi4States) {
    // save previous owner
    std::string previousOwner = state->owner;
    std::map<std::string, int> potentialOwners;
    // iterate over currently owned provinces
    for (const auto province : state->gameProvinces) {
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
    auto &newOwner = hoi4Gen.hoi4Countries.at(likeliestOwner->first);
    // give the new owner a region
    newOwner.hoi4Regions.push_back(state);
    // track the change if the owner changed
    if (newOwner.tag != previousOwner) {
      Fwg::Utils::Logging::logLine("State ", state->ID, " changed owner from ",
                                   previousOwner, " to ", newOwner.tag);
      changes.ownerChanges.insert({state, {previousOwner, newOwner.tag}});
    }
  }
  // track the change if the state was deleted
  for (auto &country : hoi4Gen.hoi4Countries) {
    if (country.second.hoi4Regions.size() == 0) {
      changes.deletedCountries.insert(country.second.tag);
    }
  }
}

Fwg::Gfx::Bitmap createCountryBitmap(const Generator &hoi4Gen) {
  Fwg::Utils::Logging::logLine("Creating Country Map from game files");
  const auto &provinceMap = hoi4Gen.fwg.provinceMap;
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
          if (pixel >= 0 && pixel < Fwg::Cfg::Values().bitmapSize) {
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
void edit(const std::string &inPath, const std::string &outputPath,
          const std::string &mapName, Generator &hoi4Gen,
          ChangeHolder &changes) {
  Fwg::Utils::Logging::logLine("Editing Countries");
  auto countryMap = Detail::createCountryBitmap(hoi4Gen);

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
        gameProvince->baseProvince->regionID > hoi4Gen.hoi4States.size())
      continue;
    auto prevRegion = hoi4Gen.hoi4States[gameProvince->baseProvince->regionID];
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
      auto &newOwner = hoi4Gen.hoi4States.at(likeliestOwner->first);
      // give the new owner a region
      newOwner->gameProvinces.push_back(gameProvince);
      // track the change if the owner changed
      if (newOwner != prevRegion) {
        Fwg::Utils::Logging::logLine("Province ", gameProvince->ID,
                                     " changed region from ", prevRegion->ID,
                                     " to ", newOwner->ID);
        changes.stateChanges.insert({gameProvince, {prevRegion, newOwner}});
      }
    }
    prevRegion->gameProvinces.push_back(gameProvince);
  }
  Fwg::Utils::Logging::logLine("Done tracking changes to states");
}

Fwg::Gfx::Bitmap createStateBitmap(const Generator &hoi4Gen) {
  Fwg::Utils::Logging::logLine("Creating State Image from game files");
  const auto &provinceMap = hoi4Gen.fwg.provinceMap;
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
  for (const auto borderPix : stateBorders) {
    states.imageData[borderPix] = Fwg::Gfx::Colour{254, 253, 255};
  }
  for (const auto borderPix : provinceBorders) {
    states.imageData[borderPix] = Fwg::Gfx::Colour{128, 253, 255};
  }
  return states;
}
} // namespace Detail

void edit(const std::string &inPath, const std::string &outputPath,
          const std::string &mapName, Generator &hoi4Gen,
          ChangeHolder &changes) {
  Fwg::Utils::Logging::logLine("Editing States");
  auto &config = Fwg::Cfg::Values();
  auto stateMap = Detail::createStateBitmap(hoi4Gen);
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
                  Fwg::Areas::AreaData& areaNewData) {
  Fwg::Areas::Provinces::readProvinceBMP(readInProvMap, heightMap,
                                         areaNewData.provinces,
                                         areaNewData.provinceColourMap);
  // now compare both maps and see which province was deleted
  for (auto i = 0; i < hoi4Gen.fwg.areas.provinces.size() + 1; i++) {
    changes.provIdMapping[i] = 0;
  }

  // now check for new provinces
  for (auto i = hoi4Gen.fwg.areas.provinces.size() + 1;
       i < areaNewData.provinces.size() + 1; i++) {
    Fwg::Utils::Logging::logLine("Added new province with ID: ", i);
    changes.newProvs.insert(i);
  }

  // track changes in IDs
  for (auto i = 0; i < hoi4Gen.fwg.areas.provinces.size(); i++) {
    if (hoi4Gen.fwg.areas.provinces[i]->pixels.size() !=
        areaNewData.provinces[i]->pixels.size()) {
      // we have SOME change
      changes.changedProvs.insert(i);
      // the province was removed
      if (!areaNewData.provinces[i]->pixels.size()) {
        changes.deletedProvs.insert(i);
        // every succeeding province has their ID modified by -1
        for (auto x = i; x < hoi4Gen.fwg.areas.provinces.size(); x++)
          changes.provIdMapping.at(x)--;
      }
    }
  }
  // now set new IDs




  
  // overwrite areas
  hoi4Gen.fwg.areas = areaNewData;
}
std::vector<std::vector<std::string>> readDefinitions(const std::string &path) {
  auto list = ParserUtils::getLinesByID(path);
  return list;
}

bool isInt(const std::string &s) {
  return !s.empty() && std::find_if(s.begin(), s.end(), [](unsigned char c) {
                         return !std::isdigit(c);
                       }) == s.end();
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
    auto tokens = ParserUtils::getTokens(substr, ' ');
    for (auto &token : tokens) {
      if (isInt(token)) {
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
    // auto provIDs = ParserUtils::getNumbers(substr, ' ');
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
          const std::string &mapName, Generator &hoi4Gen,
          ChangeHolder &changes) {
  Fwg::Utils::Logging::logLine("Editing Provinces");
  auto provMap =
      Fwg::Gfx::Bmp::load24Bit(inPath + "map//" + mapName, "provinces");
  auto heightMap = Fwg::Gfx::Bmp::load24Bit(
      inPath + "map//" + "heightmap" + ".bmp", "heightmap");
  using namespace Fwg::Areas;
  // now read in new file and compare data
  AreaData areaNewData;
  for (auto &prov : hoi4Gen.fwg.areas.provinces) {
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
  hoi4Gen.fwg.provinceMap = provMap;
  // save edited map into mod folder
  Fwg::Gfx::Bmp::save(provMap, outputPath + "map//" + mapName);
  Detail::trackChanges(hoi4Gen, provMap, changes, heightMap, areaNewData);

  // provinces are referenced: history: units, states
  // map: definition.csv, airports, adjacencies, railways, rocketsites, supply
  // nodes, unit_stacks need a rule set for every file type

  // for (const auto &edit : mapFilesToEdit) {
  //   std::string out{""};
  //   auto fileContent = ParserUtils::getLines(inPath + edit);
  //   for (auto &line : fileContent) {
  //     isProvinceID(line, "{", "}", 0, IDTransformations, deletedIDs);
  //   }
  //   for (auto &line : fileContent) {
  //     out.append(line + "\n");
  //   }
  //   ParserUtils::writeFile(outputPath + edit, out);
  // }
  // mapFilesToEdit = {std::string("//map//definition.csv")};
  // for (const auto &edit : mapFilesToEdit) {
  //  std::string out{""};
  //  auto fileContent = ParserUtils::getLines(inPath + edit);
  //  for (auto &line : fileContent) {
  //    isProvinceID(line, "", ";", 0, IDTransformations, deletedIDs);
  //  }
  //  for (auto &line : fileContent) {
  //    out.append(line + "\n");
  //  }
  //  ParserUtils::writeFile(outputPath + edit, out);
  //}

  // mapFilesToEdit = {std::string("//map//airports.txt"),
  //                   std::string("//map//rocketsites.txt")};
  // for (const auto &edit : mapFilesToEdit) {
  //   std::string out{""};
  //   auto fileContent = ParserUtils::getLines(inPath + edit);
  //   for (auto &line : fileContent) {
  //     isProvinceID(line, "{", "}", 0, IDTransformations);
  //   }
  //   for (auto &line : fileContent) {
  //     out.append(line + "\n");
  //   }
  //   ParserUtils::writeFile(outputPath + edit, out);
}
} // namespace Provinces
} // namespace Scenario::Hoi4::MapPainting
