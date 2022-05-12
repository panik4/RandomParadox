#include "generic/Flag.h"
std::map<std::string, std::vector<Colour>> Flag::colourGroups;
std::vector<std::vector<std::vector<int>>> Flag::flagTypes(7);
std::vector<std::vector<std::vector<std::string>>> Flag::flagTypeColours(7);
std::vector<std::vector<uint8_t>> Flag::flagTemplates;
std::vector<std::vector<uint8_t>> Flag::symbolTemplates;
std::vector<std::vector<std::string>> Flag::flagMetadata;
std::vector<std::vector<std::string>> Flag::symbolMetadata;
Flag::Flag() {}

Flag::Flag(const int width, const int height) : width(width), height(height) {
  image = std::vector<unsigned char>(width * height * 4, 0);
  auto randomIndex = Env::Instance().randNum() % flagTemplates.size();
  image = flagTemplates[randomIndex];
  const auto &flagInfo = flagMetadata[randomIndex];
  auto flagColourGroups = PU::getTokens(flagInfo[0], ',');
  auto symbolColourGroups = PU::getTokens(flagInfo[1], ',');

  // get the template and map all colours to indices
  std::map<Colour, std::vector<int>> colourMapping;
  for (auto i = 0; i < image.size(); i += 4) {
    Colour temp(image[i], image[i + 1], image[i + 2]);
    colourMapping[temp].push_back(i);
  }
  // determine replacements for the colours in the template.
  // pool of colours is taken from colour groups defined in metadata files
  std::vector<Colour> replacementColours;
  for (auto &colGroup : flagColourGroups) {
    const auto &colour = UtilLib::selectRandom(colourGroups[colGroup]);
    replacementColours.push_back(colour);
  }
  // now convert the old colours to the replacement colours
  // alpha values stay the same
  int colIndex = 0;
  for (const auto &mapping : colourMapping) {
    for (auto index : mapping.second) {
      image[index] = replacementColours[colIndex].getBlue();
      image[index + 1] = replacementColours[colIndex].getGreen();
      image[index + 2] = replacementColours[colIndex].getRed();
    }
    colIndex++;
  }

  // now load symbol templates
  randomIndex = Env::Instance().randNum() % symbolTemplates.size();
  auto symbol{symbolTemplates[1]};
  auto symbolInfo{symbolMetadata[randomIndex]};
  auto symbolHeightOffset = std::stod(flagInfo[4]);
  auto symbolWidthOffset = std::stod(flagInfo[3]);

  // now resize symbol
  auto reductionFactor = std::stod(flagInfo[5]);
  int newSize = 52 * reductionFactor;
  symbol = Flag::resize(newSize, newSize, symbol, 52, 52);

  // check if we want to replace the colour
  auto replaceColour = symbolInfo[0] == "true";
  replacementColours.clear();
  for (const auto &colGroup : symbolColourGroups) {
    const auto &colour = colourGroups[colGroup][Env::Instance().randNum() %
                                                colourGroups[colGroup].size()];
    replacementColours.push_back(colour);
  }
  // get the template and map all colours to indices
  colourMapping.clear();
  for (auto i = 0; i < symbol.size(); i += 4) {
    Colour temp(symbol[i], symbol[i + 1], symbol[i + 2]);
    // only if alpha is greater 0
    if (symbol[i + 3] > 0)
      colourMapping[temp].push_back(i);
  }
  colIndex = 0;
  for (const auto &mapping : colourMapping) {
    for (auto index : mapping.second) {
      // map indey from normal flag size to symbol size
      auto offset = (int)(symbolHeightOffset * 52);
      offset -= offset % 4;
      auto height = offset + (index / (int)(52 * reductionFactor * 4));

      offset = (int)((symbolWidthOffset * 328));
      offset -= offset % 4;
      auto width = offset + (index % (int)(52 * reductionFactor * 4));
      setPixel(replacementColours[colIndex], 328 * height + width);
    }
    colIndex++;
  }
}

Flag::~Flag() {}

void Flag::setPixel(const Colour colour, const int x, const int y) {
  for (auto i = 0; i < 3; i++)
    image[(x * width + y) * 4 + i] = colour.getBGR()[i];
  image[(x * width + y) * 4 + 3] = 255;
}

void Flag::setPixel(const Colour colour, const int index) {
  for (auto i = 0; i < 3; i++)
    image[index * 4 + i] = colour.getBGR()[i];
  image[index * 4 + 3] = 255;
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
  for (auto &line : lines) {
    if (!line.size())
      continue;
    auto tokens = PU::getTokens(line, ';');
    for (int i = 1; i < tokens.size(); i++) {
      auto nums = PU::getNumbers(tokens[i], ',', std::set<int>{});
      Colour c{(unsigned char)nums[0], (unsigned char)nums[1],
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
      flagTemplates.push_back(TextureWriter::readTGA(
          "resources\\flags\\flag_presets\\" + std::to_string(i) + ".tga"));
      // get line and immediately tokenize it
      flagMetadata.push_back(
          PU::getTokens(PU::getLines("resources\\flags\\flag_presets\\" +
                                     std::to_string(i) + ".txt")[0],
                        ';'));
    }
  }
}
void Flag::readSymbolTemplates() {
  for (int i = 0; i < 100; i++) {
    if (std::filesystem::exists("resources\\flags\\symbol_presets\\" +
                                std::to_string(i) + ".tga")) {
      symbolTemplates.push_back(TextureWriter::readTGA(
          "resources\\flags\\symbol_presets\\" + std::to_string(i) + ".tga"));
      // get line and immediately tokenize it
      symbolMetadata.push_back(
          PU::getTokens(PU::getLines("resources\\flags\\symbol_presets\\" +
                                     std::to_string(i) + ".txt")[0],
                        ';'));
    }
  }
}