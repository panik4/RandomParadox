#include "vic3/Vic3Parsing.h"
using namespace Fwg;
namespace Logging = Fwg::Utils::Logging;
namespace pU = Scenario::ParserUtils;
namespace Scenario::Vic3::Parsing {
namespace Writing {
void adj(const std::string &path) {
  Logging::logLine("Vic3 Parser: Map: Writing Adjacencies");
  // empty file for now
  std::string content;
  content.append("From;To;Type;Through;start_x;start_y;stop_x;stop_y;adjacency_"
                 "rule_name;Comment\n");
  pU::writeFile(path, content);
}

void defaultMap(const std::string &path,
                const std::vector<std::shared_ptr<GameProvince>> &provinces) {
  auto templateContent = pU::readFile("resources\\vic3\\map_data\\default.map");
  Logging::logLine("VIC3 Parser: Map: Default Map");
  std::string content = templateContent;
  std::string seaStarts{""};
  std::string lakes{""};
  auto seaFormatCounter = 1;
  auto lakeFormatCounter = 1;
  for (const auto &province : provinces) {
    if (province->baseProvince->sea) {
      seaStarts.append(province->toHexString() + " ");
      if (seaFormatCounter++ % 5 == 0) {
        seaStarts.append("\n\t");
      }
    } else if (province->baseProvince->isLake) {
      lakes.append(province->toHexString() + " ");
      if (lakeFormatCounter++ % 5 == 0) {
        lakes.append("\n\t");
      }
    }
  }
  pU::replaceOccurences(content, "TEMPLATE_SEA_STARTS", seaStarts);
  pU::replaceOccurences(content, "TEMPLATE_LAKES", lakes);
  pU::writeFile(path, content);
}

void heightmap(const std::string &path, const Fwg::Gfx::Bitmap &heightMap) {
  auto content = pU::readFile("resources\\vic3\\map_data\\heightmap.heightmap");
  Logging::logLine("VIC3 Parser: Map: heightmap.heightmap");
  pU::replaceOccurences(content, "template_map_x",
                        std::to_string(heightMap.bInfoHeader.biWidth));
  pU::replaceOccurences(content, "template_map_y",
                        std::to_string(heightMap.bInfoHeader.biHeight));
  pU::writeFile(path, content);
}

void stateFiles(const std::string &path,
                const std::vector<std::shared_ptr<Region>> &regions) {
  const auto templateFile =
      pU::readFile("resources\\vic3\\map_data\\state_template.txt");
  std::string file = "";
  for (const auto &region : regions) {
    auto content = templateFile;
    pU::replaceOccurences(content, "template_name", region->name);
    pU::replaceOccurences(content, "template_id",
                          std::to_string(region->ID + 1));
    std::string provinceString{""};
    for (auto prov : region->gameProvinces) {
      provinceString.append("\"" + prov->toHexString() + "\" ");
    }
    pU::replaceOccurences(content, "template_provinces", provinceString);

    int counter = 0;
    pU::replaceOccurences(content, "template_city",
                          region
                              ->gameProvinces[std::clamp(
                                  counter++, 0, (int)region->provinces.size())]
                              ->toHexString());
    pU::replaceOccurences(content, "template_port",
                          region
                              ->gameProvinces[std::clamp(
                                  counter++, 0, (int)region->provinces.size())]
                              ->toHexString());
    pU::replaceOccurences(content, "template_farm",
                          region
                              ->gameProvinces[std::clamp(
                                  counter++, 0, (int)region->provinces.size())]
                              ->toHexString());
    pU::replaceOccurences(content, "template_mine",
                          region
                              ->gameProvinces[std::clamp(
                                  counter++, 0, (int)region->provinces.size())]
                              ->toHexString());
    pU::replaceOccurences(content, "template_wood",
                          region
                              ->gameProvinces[std::clamp(
                                  counter++, 0, (int)region->provinces.size())]
                              ->toHexString());
    // TODO removed for now
    pU::replaceOccurences(content, "template_naval_exit", "");
    file.append(content);
  }
  pU::writeFile(path, file);
}

void provinceTerrains(
    const std::string &path,
    const std::vector<std::shared_ptr<GameProvince>> &provinces) {
  // x48E2A5 = "desert"
  std::string content{""};
  for (const auto &province : provinces) {
    content.append(province->toHexString());
    content.append("=\"");
    std::string terraintype;
    if (province->baseProvince->sea)
      terraintype = "ocean";
    else
      terraintype = province->terrainType;
    if (province->baseProvince->isLake) {
      terraintype = "lakes";
    }

    content.append(terraintype);
    content.append("\"\n");
  }
  pU::writeFile(path, content);
}
void writeMetadata(const std::string &path) {
  const auto templateFile = pU::readFile("resources\\vic3\\metadata.json");

  pU::writeFile(path, templateFile);
}
void strategicRegions(const std::string &path,
                      const std::vector<strategicRegion> &strategicRegions,
                      const std::vector<std::shared_ptr<Region>> &regions) {

  const auto templateFile =
      pU::readFile("resources\\vic3\\common\\strategic_regions\\template.txt");
  std::string file = "";
  for (const auto &region : strategicRegions) {
    auto content = templateFile;
    std::string states{""};
    std::string capital;
    bool capitalSelected = false;
    for (auto stateID : region.gameRegionIDs) {
      const auto &state = regions[stateID];
      states.append(" STATE_" + state->name);
      if (!capitalSelected) {
        capitalSelected = true;
        capital = state->gameProvinces[0]->toHexString();
      }
    }
    pU::replaceOccurences(content, "template_name", region.name);
    pU::replaceOccurences(content, "template_states", states);
    pU::replaceOccurences(content, "template_capital", capital);

    file.append(content);
  }
  pU::writeFile(path, file);
}
void stateHistory(const std::string &path,
                  const std::vector<std::shared_ptr<Region>> &regions) {
  auto file = pU::readFile("resources\\vic3\\common\\history\\states.txt");
  std::string stateContent = "";
  const auto stateTemplate =
      pU::readFile("resources\\vic3\\common\\history\\stateTemplate.txt");
  for (const auto &region : regions) {
    auto content = stateTemplate;
    pU::replaceOccurences(content, "templateName", region->name);
    pU::replaceOccurences(content, "templateCountry", "USA" /*TODOregion->owner*/);
    std::string provinceString{""};
    for (auto prov : region->gameProvinces) {
      provinceString.append("\"" + prov->toHexString() + "\" ");
    }
    pU::replaceOccurences(content, "templateProvinces", provinceString);

    pU::replaceOccurences(content, "templateCulture", "dixie");
    stateContent.append(content);
  }
  pU::replaceOccurences(file, "templateStateData", stateContent);

  pU::writeFile(path, file, true);
}
} // namespace Writing

namespace Reading {}
} // namespace Scenario::Vic3::Parsing