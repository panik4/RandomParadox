#include "hoi4/Hoi4MapPainting.h"

namespace Scenario::Hoi4MapPainting {

namespace Detail {
void countryBitmap(const std::string &mapPath,
                   const std::vector<Fwg::Province> &provinces,
                   const std::vector<Region> &states,
                   const Fwg::Gfx::Bitmap &provinceMap,
                   const Fwg::Utils::ColourTMap<std::string> &colourMap) {
  Fwg::Gfx::Bitmap countries(provinceMap.bInfoHeader.biWidth,
                             provinceMap.bInfoHeader.biHeight, 24);
  std::set<int> stateBorders;
  for (const auto &state : states) {
    std::set<int> statePixels;
    auto col = colourMap.valueSearch(state.owner);
    for (const auto provID : state.provinceIDs) {
      for (const auto pixel : provinces[provID].pixels) {
        countries.imageData[pixel] = col;
      }
    }
    // we only have a tag, which is the value of our map. We now need to find
    // the value in the map to get the correspondig colour
  }
  Fwg::Gfx::Bmp::save(countries, mapPath);
}

void stateBitmap(const std::string &mapPath, Fwg::Gfx::Bitmap countries,
                 const std::vector<Fwg::Province> &provinces,
                 const std::vector<Region> &states) {
  std::set<int> stateBorders;
  for (const auto &state : states) {
    std::set<int> statePixels;
    for (const auto provID : state.provinceIDs) {
      std::copy(provinces[provID].pixels.begin(),
                provinces[provID].pixels.end(),
                std::inserter(statePixels, statePixels.end()));
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
  for (const auto borderPix : stateBorders) {
    countries.imageData[borderPix] = Fwg::Gfx::Colour{254, 253, 255};
  }
  Fwg::Gfx::Bmp::save(countries, mapPath);
}
} // namespace Detail
void output(const std::string &inPath, const std::string &outputPath,
            bool multiCore, bool exportMap, const std::string &inputMap) {
  //auto provinces = Detail::readProvinceMap(inPath);
  //auto states = Detail::readStates(inPath);
  //auto colourMap = Detail::readColourMapping(inPath);
  //std::string suffix = ".bmp";
  //std::string countryMapPath = inPath + "map/" + inputMap + suffix;
  //std::string countryRegionsMapPath =
  //    inPath + "map/" + inputMap + "-regions" + suffix;
  //auto provMap =
  //    Fwg::Gfx::Bmp::load24Bit(inPath + "map/provinces.bmp", "provinces");
  //Fwg::Gfx::Bitmap ownerMap;
  //if (!std::filesystem::exists(countryMapPath)) {
  //  Fwg::Utils::Logging::logLine(
  //      "WARNING: No input Map exists in your input folder. If this is on "
  //      "purpose, a map will be generated automatically from the given files. "
  //      "Otherwise exit this program and configure it correctly");
  //  system("pause");
  //  Detail::countryBitmap(countryMapPath, provinces, states, provMap,
  //                        colourMap);
  //}
  //ownerMap = Fwg::Gfx::Bmp::load24Bit(countryMapPath, "countries");

  //using namespace Scenario::ParserUtils;
  //std::vector<std::vector<int>> regions;
  //auto stateFiles = readFilesInDirectory(inPath + "/history/states");

  //if (exportMap) {
  //  if (std::filesystem::exists(countryRegionsMapPath)) {
  //    Fwg::Utils::Logging::logLine(
  //        "WARNING: File ", countryRegionsMapPath,
  //        " will be generated from input countries map again and overwritten. "
  //        "Make sure you want that. If not, set stateExport to false. Press "
  //        "any button if you do want to continue, otherwise close the "
  //        "program.");
  //    system("pause");
  //  }
  //  if (colourMap.find(Fwg::Gfx::Colour{254, 253, 255}))
  //    Fwg::Utils::Logging::logLine(
  //        "Warning: One of your countries has the colour of the borders this "
  //        "tool generates.");

  //  Detail::stateBitmap(countryRegionsMapPath, ownerMap, provinces, states);
  //  Fwg::Utils::Logging::logLine(
  //      "Exported statebitmap as configured. You must now change the mapName "
  //      "to countries-regions.bmp, set stateExport to false and re-run.");
  //  ownerMap = Fwg::Gfx::Bmp::load24Bit(countryRegionsMapPath, "countries");
  //  return;
  //}

  //std::map<int, std::string> ownership;
  //auto ID = 0;
  //for (const auto &state : states) {
  //  std::map<std::string, int> potentialOwners;
  //  for (const auto provID : state.provinceIDs) {
  //    for (const auto pixel : provinces[provID].pixels) {
  //      const auto &col = ownerMap[pixel];
  //      if (colourMap.find(col)) {
  //        potentialOwners[colourMap[col]]++;
  //      }
  //    }
  //  }
  //  using pair_type = decltype(potentialOwners)::value_type;
  //  auto pr =
  //      std::max_element(std::begin(potentialOwners), std::end(potentialOwners),
  //                       [](const pair_type &p1, const pair_type &p2) {
  //                         return p1.second < p2.second;
  //                       });
  //  auto stateString = stateFiles[ID++];
  //  auto fileID = getValue(stateString, "id");
  //  removeCharacter(fileID, ' ');

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

namespace ProvinceEditing {
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
    // std::cout << "IN: " << content << std::endl;
    auto substr = content.substr(leftDelim, rightDelim - leftDelim);

    std::string replacementString{""};
    auto tokens = ParserUtils::getTokens(substr, ' ');
    for (auto &token : tokens) {
      if (isInt(token)) {
        auto repVal = std::stoi(token);
        if (deletedIDs.find(repVal) == deletedIDs.end()) {
          // modification mode
          std::cout << repVal << std::endl;
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

void provinceEditing(const std::string &inPath, const std::string &outputPath,
                     const std::string &mapName, const Generator &hoi4Gen) {
  auto provMap =
      Fwg::Gfx::Bmp::load24Bit(inPath + "map//" + mapName, "provinces");
  auto heightMap = Fwg::Gfx::Bmp::load24Bit(
      inPath + "map//" + "heightmap" + ".bmp", "heightmap");
  using namespace Fwg::Areas;
  // now read in enw file and compare data
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
  config.mapsToEdit.insert("provinceMap");
  Fwg::Gfx::Bmp::edit<Fwg::Gfx::Colour>("provinces.bmp", provMap, "provinceMap",
                                        config.mapsPath, config.mapsToEdit,
                                        config.editor);

  Fwg::Areas::Provinces::readProvinceBMP(
      provMap, heightMap, areaNewData.provinces, areaNewData.provinceColourMap);

  // now compare both maps and see which province was deleted

  std::map<int, int> IDTransformations;
  for (auto i = 0; i < hoi4Gen.fwg.areas.provinces.size() + 1; i++) {
    IDTransformations[i] = 0;
  }
  std::set<int> changedIDs;
  std::set<int> deletedIDs;

  for (auto i = 0; i < hoi4Gen.fwg.areas.provinces.size(); i++) {
    if (hoi4Gen.fwg.areas.provinces[i]->pixels.size() !=
        areaNewData.provinces[i]->pixels.size()) {
      // we have SOME change
      changedIDs.insert(i);
      // the province was removed
      if (!areaNewData.provinces[i]->pixels.size()) {
        deletedIDs.insert(i);
        // every succeeding province has their ID modified by -1
        for (auto x = i; x < hoi4Gen.fwg.areas.provinces.size(); x++)
          IDTransformations.at(x)--;
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
  mapFilesToEdit = {std::string("//map//definition.csv")};
  for (const auto &edit : mapFilesToEdit) {
    std::string out{""};
    auto fileContent = ParserUtils::getLines(inPath + edit);
    for (auto &line : fileContent) {
      isProvinceID(line, "", ";", 0, IDTransformations, deletedIDs);
    }
    for (auto &line : fileContent) {
      out.append(line + "\n");
    }
    ParserUtils::writeFile(outputPath + edit, out);
  }

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
} // namespace ProvinceEditing
} // namespace Scenario::Hoi4MapPainting