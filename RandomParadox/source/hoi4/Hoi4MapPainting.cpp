#include "hoi4/Hoi4MapPainting.h"

namespace Scenario::Hoi4MapPainting {

namespace Detail {

// reads a text file containing colour->tag relations
// reads a bmp containing colours
Fwg::Utils::ColourTMap<std::string> readColourMapping(const std::string &path) {
  using namespace Scenario::ParserUtils;
  Fwg::Utils::ColourTMap<std::string> colourMap;
  auto mappings = readFile(path + "//common/countries/colors.txt");
  std::string countryColour;
  do {
    countryColour =
        removeSurroundingBracketBlockFromLineBreak(mappings, "color = ");
    if (countryColour.size()) {
      auto tag = countryColour.substr(1, 3);
      auto colourString = getValue(countryColour, "color_ui");
      auto hsv = getBracketBlockContent(colourString, "hsv");
      std::vector<int> rgb(3);
      if (colourString.find("rgb") != std::string::npos) {
        rgb = getNumberBlock(colourString, "rgb");
      } else if (hsv.size()) {
        auto hsvd = getTokens(hsv, ' ');
        for (int i = 0; i < hsvd.size(); i++) {
          if (!hsvd[i].size())
            hsvd.erase(hsvd.begin() + i);
        }

        std::vector<double> hsvv;
        hsvv.push_back(std::stod(hsvd[0]) * 360.0);
        hsvv.push_back(std::stod(hsvd[1]));
        hsvv.push_back(std::stod(hsvd[2]));
        auto C = hsvv[2] * hsvv[1];
        // C × (1 - |(H / 60°) mod 2 - 1|)
        auto X = C * (1.0 - abs(std::fmod((hsvv[0] / 60), 2.0) - 1.0));
        auto m = hsvv[2] - C;
        //  ((R'+m)×255, (G'+m)×255, (B'+m)×255)
        rgb[0] = static_cast<int>((C + m) * 255.0) % 255;
        rgb[1] = static_cast<int>((X + m) * 255.0) % 255;
        rgb[2] = static_cast<int>((0.0 + m) * 255.0) % 255;

        if (hsvv[0] < 60) {
        } else if (hsvv[0] < 120) {
          std::swap(rgb[1], rgb[0]);
        } else if (hsvv[0] < 180) {
          std::swap(rgb[2], rgb[0]);
          std::swap(rgb[2], rgb[1]);
        } else if (hsvv[0] < 240) {
          std::swap(rgb[2], rgb[0]);
        } else if (hsvv[0] < 300) {
          std::swap(rgb[0], rgb[1]);
          std::swap(rgb[1], rgb[2]);
        } else {
          std::swap(rgb[1], rgb[2]);
        }
      }
      colourMap.setValue({static_cast<unsigned char>(rgb[0]),
                          static_cast<unsigned char>(rgb[1]),
                          static_cast<unsigned char>(rgb[2])},
                         tag);
    }
  } while (countryColour.size());
  return colourMap;
}
// states are where tags are written down, expressing ownership of the map
// read them in from path, map province IDs against states
std::vector<Region> readStates(const std::string &path) {
  using namespace Scenario::ParserUtils;
  std::vector<Region> regions;
  auto states = readFilesInDirectory(path + "/history/states");

  for (auto &state : states) {
    Region reg;
    auto tag = getValue(state, "owner");
    removeCharacter(tag, ' ');
    reg.owner = tag;
    reg.provinceIDs = getNumberBlock(state, "provinces");
    regions.push_back(reg);
  }
  return regions;
}
// get the bmp file info and extract the respective IDs from definition.csv
std::vector<Fwg::Province> readProvinceMap(const std::string &path) {
  using namespace Scenario::ParserUtils;
  auto provMap =
      Fwg::Gfx::Bmp::load24Bit(path + "map/provinces.bmp", "provinces");
  auto definition = getLines(path + "map/definition.csv");
  Fwg::Utils::ColourTMap<Fwg::Province> provinces;
  for (const auto &line : definition) {
    auto nums = getNumbers(line, ';', {0, 1, 2, 3});
    provinces.setValue({static_cast<unsigned char>(nums[1]),
                        static_cast<unsigned char>(nums[2]),
                        static_cast<unsigned char>(nums[3])},
                       {nums[0],
                        {static_cast<unsigned char>(nums[1]),
                         static_cast<unsigned char>(nums[2]),
                         static_cast<unsigned char>(nums[3])},
                        false});
  }
  std::vector<Fwg::Province> retProvs(definition.size());
  for (auto i = 0; i < provMap.imageData.size(); i++) {
    const auto colour = provMap[i];
    provinces[colour].pixels.push_back(i);
    retProvs[provinces[colour].ID].pixels.push_back(i);
  }
  return retProvs;
}

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
  auto provinces = Detail::readProvinceMap(inPath);
  auto states = Detail::readStates(inPath);
  auto colourMap = Detail::readColourMapping(inPath);
  std::string suffix = ".bmp";
  std::string countryMapPath = inPath + "map/" + inputMap + suffix;
  std::string countryRegionsMapPath =
      inPath + "map/" + inputMap + "-regions" + suffix;
  auto provMap =
      Fwg::Gfx::Bmp::load24Bit(inPath + "map/provinces.bmp", "provinces");
  Fwg::Gfx::Bitmap ownerMap;
  if (!std::filesystem::exists(countryMapPath)) {
    Fwg::Utils::Logging::logLine(
        "WARNING: No input Map exists in your input folder. If this is on "
        "purpose, a map will be generated automatically from the given files. "
        "Otherwise exit this program and configure it correctly");
    system("pause");
    Detail::countryBitmap(countryMapPath, provinces, states, provMap,
                          colourMap);
  }
  ownerMap = Fwg::Gfx::Bmp::load24Bit(countryMapPath, "countries");

  using namespace Scenario::ParserUtils;
  std::vector<std::vector<int>> regions;
  auto stateFiles = readFilesInDirectory(inPath + "/history/states");

  if (exportMap) {
    if (std::filesystem::exists(countryRegionsMapPath)) {
      Fwg::Utils::Logging::logLine(
          "WARNING: File ", countryRegionsMapPath,
          " will be generated from input countries map again and overwritten. "
          "Make sure you want that. If not, set stateExport to false. Press "
          "any button if you do want to continue, otherwise close the "
          "program.");
      system("pause");
    }
    if (colourMap.find(Fwg::Gfx::Colour{254, 253, 255}))
      Fwg::Utils::Logging::logLine(
          "Warning: One of your countries has the colour of the borders this "
          "tool generates.");

    Detail::stateBitmap(countryRegionsMapPath, ownerMap, provinces, states);
    Fwg::Utils::Logging::logLine(
        "Exported statebitmap as configured. You must now change the mapName "
        "to countries-regions.bmp, set stateExport to false and re-run.");
    ownerMap = Fwg::Gfx::Bmp::load24Bit(countryRegionsMapPath, "countries");
    return;
  }

  std::map<int, std::string> ownership;
  auto ID = 0;
  for (const auto &state : states) {
    std::map<std::string, int> potentialOwners;
    for (const auto provID : state.provinceIDs) {
      for (const auto pixel : provinces[provID].pixels) {
        const auto &col = ownerMap[pixel];
        if (colourMap.find(col)) {
          potentialOwners[colourMap[col]]++;
        }
      }
    }
    using pair_type = decltype(potentialOwners)::value_type;
    auto pr =
        std::max_element(std::begin(potentialOwners), std::end(potentialOwners),
                         [](const pair_type &p1, const pair_type &p2) {
                           return p1.second < p2.second;
                         });
    auto stateString = stateFiles[ID++];
    auto fileID = getValue(stateString, "id");
    removeCharacter(fileID, ' ');

    std::string cores{""};
    if (multiCore) {
      // if multicore, give every candidate a core
      for (const auto &potentialOwner : potentialOwners) {
        cores.append("\n\t\tadd_core_of = " + potentialOwner.first + "\n");
      }
    }

    if (pr != std::end(potentialOwners)) {
      // first remove all cores
      while (replaceLine(stateString, "add_core_of=", "") ||
             replaceLine(stateString, "add_core_of =", "")) {
      };
      // now set owner and add cores
      replaceLine(stateString, "owner=", "owner = " + pr->first + cores);
      replaceLine(stateString, "owner =", "owner = " + pr->first + cores);
    }
    writeFile(outputPath + "//history//states//" + fileID + ".txt",
              stateString);
  }
}

} // namespace Scenario::Hoi4MapPainting