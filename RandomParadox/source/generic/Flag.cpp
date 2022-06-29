#include "generic/Flag.h"

namespace Scenario::Gfx {
using namespace Fwg;
namespace PU = Scenario::ParserUtils;
std::map<std::string, std::vector<Fwg::Gfx::Colour>> Flag::colourGroups;
std::vector<std::vector<std::vector<int>>> Flag::flagTypes(7);
std::vector<std::vector<std::vector<std::string>>> Flag::flagTypeColours(7);
std::vector<std::vector<uint8_t>> Flag::flagTemplates;
std::vector<std::vector<uint8_t>> Flag::symbolTemplates;
std::vector<FlagInfo> Flag::flagMetadata;
std::vector<SymbolInfo> Flag::symbolMetadata;
Flag::Flag() {}

Flag::Flag(const int width, const int height) : width(width), height(height) {
  image = std::vector<unsigned char>(width * height * 4, 0);
  auto randomIndex = RandNum::randNum() % flagTemplates.size();
  image = flagTemplates[randomIndex];
  const auto &flagInfo = flagMetadata[randomIndex];
  // get the template and map all colours to indices
  std::map<Fwg::Gfx::Colour, std::vector<int>> colourMapping;
  for (auto i = 0; i < image.size(); i += 4) {
    Fwg::Gfx::Colour temp(image[i], image[i + 1], image[i + 2]);
    colourMapping[temp].push_back(i);
  }
  // determine replacements for the colours in the template.
  // pool of colours is taken from colour groups defined in metadata files
  std::vector<Fwg::Gfx::Colour> replacementColours;
  for (auto &colGroup : flagInfo.flagColourGroups) {
    const auto &colour = Utils::selectRandom(colourGroups[colGroup]);
    replacementColours.push_back(colour);
  }

  // now convert the old colours to the replacement colours
  // alpha values stay the same
  int colIndex = 0;
  for (const auto &mapping : colourMapping) {
    for (auto index : mapping.second)
      setPixel(replacementColours[colIndex], index);
    colIndex++;
  }
  // now load symbol templates
  randomIndex = RandNum::randNum() % symbolTemplates.size();
  auto symbol{symbolTemplates[randomIndex]};
  auto symbolInfo{symbolMetadata[randomIndex]};

  // now resize symbol
  int newSize = 52 * flagInfo.reductionFactor;
  symbol = Flag::resize(newSize, newSize, symbol, 52, 52);

  replacementColours.clear();
  for (const auto &colGroup : flagInfo.symbolColourGroups) {
    const auto &colour = colourGroups[colGroup][RandNum::randNum() %
                                                colourGroups[colGroup].size()];
    replacementColours.push_back(colour);
  }
  // get the template and map all colours to indices
  colourMapping.clear();
  for (auto i = 0; i < symbol.size(); i += 4) {
    Fwg::Gfx::Colour temp(symbol[i], symbol[i + 1], symbol[i + 2]);
    // only if alpha is greater 0
    if (symbol[i + 3] > 0)
      colourMapping[temp].push_back(i);
  }
  colIndex = 0;
  const int lineSize = 328;
  const int symbolLineSize = 52 * 4;
  for (const auto &mapping : colourMapping) {
    for (auto index : mapping.second) {
      // map index from normal flag size to symbol size
      auto offset = (int)(flagInfo.symbolHeightOffset * 52);
      offset -= offset % 4;
      auto height =
          offset + (index / (int)(symbolLineSize * flagInfo.reductionFactor));
      offset = (int)((flagInfo.symbolWidthOffset * lineSize));
      offset -= offset % 4;
      auto width = offset + (index % (int)(52 * flagInfo.reductionFactor * 4));
      setPixel(replacementColours[colIndex], lineSize * height + width);
    }
    colIndex++;
  }
  return;
}

Flag::~Flag() {}

void Flag::setPixel(const Fwg::Gfx::Colour colour, const int x, const int y) {
  if (Utils::inRange(0, width * height * 4 + 3, (x * width + y) * 4 + 3)) {
    for (auto i = 0; i < 3; i++)
      image[(x * width + y) * 4 + i] = colour.getBGR()[i];
    image[(x * width + y) * 4 + 3] = 255;
  }
}

void Flag::setPixel(const Fwg::Gfx::Colour colour, const int index) {
  if (Utils::inRange(0, width * height * 4 + 3, index)) {
    for (auto i = 0; i < 3; i++)
      image[index + i] = colour.getBGR()[i];
    image[index + 3] = 255;
  }
}

std::vector<unsigned char> Flag::getFlag() const { return image; }

std::vector<uint8_t> Flag::resize(const int width, const int height) const {
  auto resized = std::vector<unsigned char>(width * height * 4, 0);
  const auto factor = this->width / width;
  for (int h = 0; h < height; h++) {
    for (int w = 0; w < width; w++) {
      auto colourmapIndex = factor * h * this->width + factor * w;
      colourmapIndex *= 4;
      resized[(h * width + w) * 4] = image[colourmapIndex];
      resized[(h * width + w) * 4 + 1] = image[colourmapIndex + 1];
      resized[(h * width + w) * 4 + 2] = image[colourmapIndex + 2];
      resized[(h * width + w) * 4 + 3] = image[colourmapIndex + 3];
    }
  }
  return resized;
}

std::vector<uint8_t> Flag::resize(const int width, const int height,
                                  const std::vector<unsigned char> tImage,
                                  const int inWidth, const int inHeight) {
  auto resized = std::vector<unsigned char>(width * height * 4, 0);
  const auto factor = inWidth / width;
  for (auto h = 0; h < height; h++) {
    for (auto w = 0; w < width; w++) {
      auto colourmapIndex = factor * h * inWidth + factor * w;
      colourmapIndex *= 4;
      for (auto i = 0; i < 4; i++)
        resized[(h * width + w) * 4 + i] = tImage[colourmapIndex + i];
    }
  }
  return resized;
}

void Flag::readColourGroups() {
  auto lines = PU::getLines("resources\\flags\\colour_groups.txt");
  for (const auto &line : lines) {
    if (!line.size())
      continue;
    auto tokens = PU::getTokens(line, ';');
    for (auto i = 1; i < tokens.size(); i++) {
      const auto nums = PU::getNumbers(tokens[i], ',', std::set<int>{});
      Fwg::Gfx::Colour c{(unsigned char)nums[0], (unsigned char)nums[1],
               (unsigned char)nums[2]};
      colourGroups[tokens[0]].push_back(c);
    }
  }
}

void Flag::readFlagTypes() {
  auto lines = PU::getLines("resources\\flags\\flag_types.txt");
  for (const auto &line : lines) {
    if (!line.size())
      continue;
    auto tokens = PU::getTokens(line, ';');
    const auto flagType = stoi(tokens[0]);
    const auto flagTypeID = flagTypes[flagType].size();
    const auto symbols = PU::getTokens(tokens[1], ',');
    const auto colourGroupStrings = PU::getTokens(tokens[2], ',');
    flagTypes[flagType].push_back(std::vector<int>{});
    flagTypeColours[flagType].push_back(std::vector<std::string>{});
    for (const auto &symbolRange : symbols) {
      const auto &rangeTokens =
          PU::getNumbers(symbolRange, '-', std::set<int>{});
      for (auto x = rangeTokens[0]; x <= rangeTokens[1]; x++)
        flagTypes[flagType][flagTypeID].push_back(x);
    }
    for (const auto &cGroup : colourGroupStrings)
      flagTypeColours[flagType][flagTypeID].push_back(cGroup);
  }
}

void Flag::readFlagTemplates() {
  for (auto i = 0; i < 100; i++) {
    if (std::filesystem::exists("resources\\flags\\flag_presets\\" +
                                std::to_string(i) + ".tga")) {
      flagTemplates.push_back(Gfx::Textures::readTGA(
          "resources\\flags\\flag_presets\\" + std::to_string(i) + ".tga"));
      // get line and immediately tokenize it
      auto tokens =
          PU::getTokens(PU::getLines("resources\\flags\\flag_presets\\" +
                                     std::to_string(i) + ".txt")[0],
                        ';');
      flagMetadata.push_back({PU::getTokens(tokens[0], ','),
                              PU::getTokens(tokens[1], ','), stod(tokens[3]),
                              stod(tokens[4]), stod(tokens[5])});
    }
  }
}
void Flag::readSymbolTemplates() {
  for (int i = 0; i < 100; i++) {
    if (std::filesystem::exists("resources\\flags\\symbol_presets\\" +
                                std::to_string(i) + ".tga")) {
      symbolTemplates.push_back(Gfx::Textures::readTGA(
          "resources\\flags\\symbol_presets\\" + std::to_string(i) + ".tga"));
      // get line and immediately tokenize it
      auto tokens =
          PU::getTokens(PU::getLines("resources\\flags\\symbol_presets\\" +
                                     std::to_string(i) + ".txt")[0],
                        ';');
      symbolMetadata.push_back({tokens[0] == "true"});
    }
  }
}
} // namespace Scenario::Gfx