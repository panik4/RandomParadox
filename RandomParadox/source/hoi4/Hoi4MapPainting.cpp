#include "hoi4/Hoi4MapPainting.h"

namespace Scenario::Hoi4MapPainting {

// reads a text file containing colour->tag relations
// reads a bmp containing colours
Fwg::Utils::ColourTMap<std::string> readColourMapping(const std::string &path) {
  using namespace Scenario::ParserUtils;
  Fwg::Utils::ColourTMap<std::string> colourMap;
  auto mappings = getLines(path + "map/colourMapping.txt");
  for (const auto &line : mappings) {
    auto tokens = getTokens(line, ';');
    Fwg::Gfx::Colour colour = {
        static_cast<unsigned char>(std::stoi(tokens[0])),
        static_cast<unsigned char>(std::stoi(tokens[1])),
        static_cast<unsigned char>(std::stoi(tokens[2]))};
    colourMap.setValue(colour, tokens[3]);
  }
  return colourMap;
}
// states are where tags are written down, expressing ownership of the map
// read them in from path, map province IDs against states
std::vector<std::vector<int>> readStates(const std::string &path) {
  using namespace Scenario::ParserUtils;
  std::vector<std::vector<int>> regions;
  auto states = readFilesInDirectory(path + "/history/states");

  for (auto &state : states) {
    // auto provString = getBracketBlockContent(state, "provinces = {");
    std::vector<int> provIDs = getNumberBlock(state, "provinces");
    regions.push_back(provIDs);
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

void Hoi4MapPainting::output(std::vector<std::vector<int>> states,
                             std::vector<Fwg::Province> provinces,
                             const std::string &inPath,
                             const std::string &outputPath) {

  auto colourMap = readColourMapping(inPath);
  auto provMap =
      Fwg::Gfx::Bmp::load24Bit(inPath + "map/provinces.bmp", "provinces");
  auto countryMap =
      Fwg::Gfx::Bmp::load24Bit(inPath + "map/countries.bmp", "countries");
  using namespace Scenario::ParserUtils;
  std::vector<std::vector<int>> regions;
  auto stateFiles = readFilesInDirectory(inPath + "/history/states");

  std::map<int, std::string> ownership;
  int ID = 0;

  for (const auto &state : states) {
    std::map<std::string, int> potentialOwners;
    for (auto &provID : state) {
      for (auto &pixel : provinces[provID].pixels) {
        auto col = countryMap[pixel];
        if (colourMap.find(col)) {
          auto tag = colourMap[col];
          potentialOwners[tag]++;
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
    if (pr!=std::end(potentialOwners)) {
      std::cout << pr->first << std::endl;
      std::cout << pr->second << std::endl;
      replaceLine(stateString, "owner =", "owner = " + pr->first);
      replaceLine(stateString, "owner=", "owner = " + pr->first);
    }
    writeFile(outputPath + "//history//states//" + fileID + ".txt",
              stateString);
  }
}

} // namespace Scenario::Hoi4MapPainting