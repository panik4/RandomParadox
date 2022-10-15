#include "hoi4/Hoi4MapPainting.h"

namespace Scenario::Hoi4::MapPainting {

namespace Detail {
void stateBitmap(const std::string &mapPath, Fwg::Gfx::Bitmap countries,
                 const std::vector<Fwg::Province> &provinces,
                 const std::vector<Region> &states) {
  // std::set<int> stateBorders;
  // for (const auto &state : states) {
  //   std::set<int> statePixels;
  //   for (const auto provID : state.provinceIDs) {
  //     std::copy(provinces[provID].pixels.begin(),
  //               provinces[provID].pixels.end(),
  //               std::inserter(statePixels, statePixels.end()));
  //   }
  //   for (const auto pixel : statePixels) {
  //     std::array<int, 4> newPixels = {pixel + 1, pixel - 1,
  //                                     pixel + countries.bInfoHeader.biWidth,
  //                                     pixel - countries.bInfoHeader.biWidth};
  //     for (const auto newPix : newPixels) {
  //       if (statePixels.find(newPix) == statePixels.end()) {
  //         stateBorders.insert(pixel);
  //       }
  //     }
  //   }
  // }
  // for (const auto borderPix : stateBorders) {
  //   countries.imageData[borderPix] = Fwg::Gfx::Colour{254, 253, 255};
  // }
  // Fwg::Gfx::Bmp::save(countries, mapPath);
}
} // namespace Detail
namespace Countries {
namespace Detail {
void trackChanges(Generator &hoi4Gen, const Fwg::Gfx::Bitmap readInCountryMap,
                  ChangeHolder &changes) {
  for (auto &country : hoi4Gen.hoi4Countries) {
    country.second.hoi4Regions.clear();
    country.second.ownedRegions.clear();
  }

  std::map<int, std::string> ownership;
  auto ID = 0;
  for (const auto &state : hoi4Gen.hoi4States) {
    std::string previousOwner = state->owner;
    std::map<std::string, int> potentialOwners;
    for (const auto province : state->gameProvinces) {
      for (const auto pixel : province->baseProvince->pixels) {
        const auto &col = readInCountryMap[pixel];
        if (hoi4Gen.colourMap.find(col)) {
          potentialOwners[hoi4Gen.colourMap[col]]++;
        }
      }
    }
    using pair_type = decltype(potentialOwners)::value_type;
    auto likeliestOwner =
        std::max_element(std::begin(potentialOwners), std::end(potentialOwners),
                         [](const pair_type &p1, const pair_type &p2) {
                           return p1.second < p2.second;
                         });

    auto &newOwner = hoi4Gen.hoi4Countries.at(likeliestOwner->first);
    newOwner.hoi4Regions.push_back(state);
    if (newOwner.tag != previousOwner) {

      Fwg::Utils::Logging::logLine("State ", state->ID, " changed owner from ",
                                   previousOwner, " to ", newOwner.tag);
      changes.countryTagMapping.insert({previousOwner, newOwner.tag});
    }
  }
  for (auto &country : hoi4Gen.hoi4Countries) {
    if (country.second.hoi4Regions.size() == 0) {
      changes.deletedCountries.insert(country.second.tag);
    }
  }
}

Fwg::Gfx::Bitmap createCountryBitmap(const Generator &hoi4Gen) {
  const auto &provinceMap = hoi4Gen.fwg.provinceMap;
  Fwg::Gfx::Bitmap countries(provinceMap.bInfoHeader.biWidth,
                             provinceMap.bInfoHeader.biHeight, 24);
  std::set<int> stateBorders;
  for (const auto &country : hoi4Gen.hoi4Countries) {
    for (const auto &state : country.second.hoi4Regions) {
      std::set<int> statePixels;
      auto col = country.second.colour;
      for (const auto &prov : state->gameProvinces) {
        for (const auto pixel : prov->baseProvince->pixels) {
          if (pixel >= 0 && pixel < Fwg::Cfg::Values().bitmapSize) {
            statePixels.insert(pixel);
            countries.imageData[pixel] = col;
          }
        }
      }
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
  for (const auto borderPix : stateBorders) {
    countries.imageData[borderPix] = Fwg::Gfx::Colour{254, 253, 255};
  }
  return countries;
}

} // namespace Detail
void edit(const std::string &inPath, const std::string &outputPath,
          const std::string &mapName, Generator &hoi4Gen,
          ChangeHolder &changes) {

  // auto colourMap =
  // Scenario::Hoi4::Parsing::Reading::readColourMapping(inPath);
  auto countryMap = Detail::createCountryBitmap(hoi4Gen);

  auto &config = Fwg::Cfg::Values();
  // edit and save in editedMaps directory
  Fwg::Gfx::Bmp::edit<Fwg::Gfx::Colour>("countries.bmp", countryMap,
                                        "countryMap", config.mapsPath,
                                        config.mapsToEdit, config.editor);
  Detail::trackChanges(hoi4Gen, countryMap, changes);

  // we now have a changed country bitmap, which allows us to track the done
  // changes it makes sense to overlay the new map over the new state of
  // provinces and states

  // determine which states are now in different hands, and who should have
  // them, and cores...

  // auto provinces = Detail::readProvinceMap(inPath);
  // auto states = Detail::readStates(inPath);
  // auto colourMap = Detail::readColourMapping(inPath);
  // std::string suffix = ".bmp";
  // std::string countryMapPath = inPath + "map/" + inputMap + suffix;
  // std::string countryRegionsMapPath =
  //     inPath + "map/" + inputMap + "-regions" + suffix;
  // auto provMap =
  //     Fwg::Gfx::Bmp::load24Bit(inPath + "map/provinces.bmp", "provinces");
  // Fwg::Gfx::Bitmap ownerMap;
  // if (!std::filesystem::exists(countryMapPath)) {
  //   Fwg::Utils::Logging::logLine(
  //       "WARNING: No input Map exists in your input folder. If this is on "
  //       "purpose, a map will be generated automatically from the given files.
  //       " "Otherwise exit this program and configure it correctly");
  //   system("pause");
  //   Detail::countryBitmap(countryMapPath, provinces, states, provMap,
  //                         colourMap);
  // }
  // ownerMap = Fwg::Gfx::Bmp::load24Bit(countryMapPath, "countries");

  // using namespace Scenario::ParserUtils;
  // std::vector<std::vector<int>> regions;
  // auto stateFiles = readFilesInDirectory(inPath + "/history/states");

  // if (exportMap) {
  //   if (std::filesystem::exists(countryRegionsMapPath)) {
  //     Fwg::Utils::Logging::logLine(
  //         "WARNING: File ", countryRegionsMapPath,
  //         " will be generated from input countries map again and overwritten.
  //         " "Make sure you want that. If not, set stateExport to false. Press
  //         " "any button if you do want to continue, otherwise close the "
  //         "program.");
  //     system("pause");
  //   }
  //   if (colourMap.find(Fwg::Gfx::Colour{254, 253, 255}))
  //     Fwg::Utils::Logging::logLine(
  //         "Warning: One of your countries has the colour of the borders this
  //         " "tool generates.");

  //  Detail::stateBitmap(countryRegionsMapPath, ownerMap, provinces, states);
  //  Fwg::Utils::Logging::logLine(
  //      "Exported statebitmap as configured. You must now change the mapName "
  //      "to countries-regions.bmp, set stateExport to false and re-run.");
  //  ownerMap = Fwg::Gfx::Bmp::load24Bit(countryRegionsMapPath, "countries");
  //  return;
  //}

  // std::map<int, std::string> ownership;
  // auto ID = 0;
  // for (const auto &state : states) {
  //   std::map<std::string, int> potentialOwners;
  //   for (const auto provID : state.provinceIDs) {
  //     for (const auto pixel : provinces[provID].pixels) {
  //       const auto &col = ownerMap[pixel];
  //       if (colourMap.find(col)) {
  //         potentialOwners[colourMap[col]]++;
  //       }
  //     }
  //   }
  //   using pair_type = decltype(potentialOwners)::value_type;
  //   auto pr =
  //       std::max_element(std::begin(potentialOwners),
  //       std::end(potentialOwners),
  //                        [](const pair_type &p1, const pair_type &p2) {
  //                          return p1.second < p2.second;
  //                        });
  //   auto stateString = stateFiles[ID++];
  //   auto fileID = getValue(stateString, "id");
  //   removeCharacter(fileID, ' ');

  //  std::string cores{""};
  //  if (multiCore) {
  //    // if multicore, give every candidate a core
  //    for (const auto &potentialOwner : potentialOwners) {
  //      cores.append("\n\t\tadd_core_of = " + potentialOwner.first + "\n");
  //    }
  //  }

  //  if (pr != std::end(potentialOwners)) {
  //    // first remove all cores
  //    while (replaceLine(stateString, "add_core_of=", "") ||
  //           replaceLine(stateString, "add_core_of =", "")) {
  //    };
  //    // now set owner and add cores
  //    replaceLine(stateString, "owner=", "owner = " + pr->first + cores);
  //    replaceLine(stateString, "owner =", "owner = " + pr->first + cores);
  //  }
  //  writeFile(outputPath + "//history//states//" + fileID + ".txt",
  //            stateString);
  //}
}
} // namespace Countries

namespace Provinces {
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

void edit(const std::string &inPath, const std::string &outputPath,
          const std::string &mapName, Generator &hoi4Gen,
          ChangeHolder &changes) {
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

  Fwg::Areas::Provinces::readProvinceBMP(
      provMap, heightMap, areaNewData.provinces, areaNewData.provinceColourMap);
  // save edited map into mod folder
  Fwg::Gfx::Bmp::save(provMap, outputPath + "map//" + mapName);
  hoi4Gen.fwg.provinceMap = provMap;

  // now compare both maps and see which province was deleted
  for (auto i = 0; i < hoi4Gen.fwg.areas.provinces.size() + 1; i++) {
    changes.provIdMapping[i] = 0;
  }

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
  // provinces are referenced: history: units, states
  // map: definition.csv, airports, adjacencies, railways, rocketsites, supply
  // nodes, unit_stacks need a rule set for every file type

  // start with reading in files, and checking for certain rules:
  std::vector<std::string> mapFilesToEdit = {
      std::string("//map//airports.txt"),
      std::string("//map//rocketsites.txt")};
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