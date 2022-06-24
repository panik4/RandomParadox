#include "generic/ScenarioGenerator.h"
namespace Logging = Fwg::Utils::Logging;
namespace Scenario {
using namespace Fwg::Gfx;
Generator::Generator(Fwg::FastWorldGenerator &fwg) : fwg(fwg) {
  Gfx::Flag::readColourGroups();
  Gfx::Flag::readFlagTypes();
  Gfx::Flag::readFlagTemplates();
  Gfx::Flag::readSymbolTemplates();
}

Generator::~Generator() {}

void Generator::loadRequiredResources(const std::string &gamePath) {
  bitmaps["provinces"] = ResourceLoading::loadProvinceMap(gamePath);
  bitmaps["heightmap"] = ResourceLoading::loadHeightMap(gamePath);
  Fwg::Gfx::Bitmap::bufferBitmap("provinces", bitmaps["provinces"]);
}

void Generator::generateWorld() {
  mapTerrain();
  generatePopulations();
  generateDevelopment();
}

void Generator::mapContinents() {
  Logging::logLine("Mapping Continents");
  for (const auto &continent : fwg.provinceGenerator.continents) {
    // we copy the fwg continents by choice, to leave them untouched
    pdoxContinents.push_back(PdoxContinent(continent));
  }
}

void Generator::mapRegions() {
  Logging::logLine("Mapping Regions");
  for (auto &region : fwg.provinceGenerator.regions) {
    std::sort(region.provinces.begin(), region.provinces.end(),
              [](const Fwg::Province *a, const Fwg::Province *b) {
                return (*a < *b);
              });
    Region gR(region);
    for (auto &baseRegion : gR.neighbours)
      gR.neighbours.push_back(baseRegion);
    // generate random name for region
    gR.name = nG.generateName();
    // now create gameprovinces from FastWorldGen provinces
    for (auto &province : gR.provinces) {
      GameProvince gP(province);
      // also copy neighbours
      for (auto &baseProvinceNeighbour : gP.baseProvince->neighbours)
        gP.neighbours.push_back(baseProvinceNeighbour);
      // give name to province
      gP.name = nG.generateName();
      gR.gameProvinces.push_back(gP);
      gameProvinces.push_back(gP);
    }
    // save game region
    gameRegions.push_back(gR);
  }
  // check if we have the same amount of gameProvinces as FastWorldGen provinces
  if (gameProvinces.size() != fwg.provinceGenerator.provinces.size())
    throw(std::exception("Fatal: Lost provinces, terminating"));
  if (gameRegions.size() != fwg.provinceGenerator.regions.size())
    throw(std::exception("Fatal: Lost regions, terminating"));
  // sort by gameprovince ID
  std::sort(gameProvinces.begin(), gameProvinces.end());
}

void Generator::generatePopulations() {
  Logging::logLine("Generating Population");
  auto &config = Fwg::Env::Instance();
  const auto &popMap = Bitmap::findBitmapByKey("population");
  const auto &cityMap = Bitmap::findBitmapByKey("cities");
  for (auto &c : countries)
    for (auto &gR : c.second.ownedRegions)
      for (auto &gProv : gameRegions[gR].gameProvinces) {
        // calculate the population factor
        gProv.popFactor =
            0.1 + popMap.getColourAtIndex(
                      gProv.baseProvince->position.weightedCenter) /
                      config.namedColours["population"];
        int cityPixels = 0;
        // calculate share of province that is a city
        for (auto pix : gProv.baseProvince->pixels)
          if (cityMap.getColourAtIndex(pix).isShadeOf(
                  config.namedColours["cities"]))
            cityPixels++;
        gProv.cityShare =
            (double)cityPixels / gProv.baseProvince->pixels.size();
      }
}

void Generator::generateDevelopment() {
  // high pop-> high development
  // high city share->high dev
  // terrain type?
  // .....
  Logging::logLine("Generating State Development");
  auto &config = Fwg::Env::Instance();
  const auto &cityBMP = Bitmap::findBitmapByKey("cities");
  for (auto &c : countries)
    for (auto &gR : c.second.ownedRegions)
      for (auto &gameProv : gameRegions[gR].gameProvinces) {
        auto cityDensity = 0.0;
        // calculate development with density of city and population factor
        if (gameProv.baseProvince->cityPixels.size())
          cityDensity =
              cityBMP.getColourAtIndex(gameProv.baseProvince->cityPixels[0]) /
              config.namedColours["cities"];
        gameProv.devFactor =
            std::clamp(0.2 + 0.5 * gameProv.popFactor +
                           1.0 * gameProv.cityShare * cityDensity,
                       0.0, 1.0);
      }
}

void Generator::mapTerrain() {
  auto &config = Fwg::Env::Instance();
  const auto &namedColours = config.namedColours;
  const auto &climateMap = Bitmap::findBitmapByKey("climate");
  Bitmap typeMap(climateMap.bInfoHeader.biWidth,
                 climateMap.bInfoHeader.biHeight, 24);
  Logging::logLine("Mapping Terrain");
  std::vector<std::string> targetTypes{"plains",   "forest", "marsh", "hills",
                                       "mountain", "desert", "urban", "jungle"};

  for (auto &c : countries)
    for (auto &gameRegion : c.second.ownedRegions)
      for (auto &gameProv : gameRegions[gameRegion].gameProvinces) {
        std::map<Fwg::Gfx::Colour, int> colourPrevalence;
        for (auto &pix : gameProv.baseProvince->pixels) {
          if (colourPrevalence[climateMap.getColourAtIndex(pix)])
            colourPrevalence[climateMap.getColourAtIndex(pix)]++;
          else
            colourPrevalence[climateMap.getColourAtIndex(pix)] = 1;
        }
        // find the most prevalent colour in province, which sets the terrain
        // type
        using pair_type = decltype(colourPrevalence)::value_type;
        auto pr = std::max_element(
            std::begin(colourPrevalence), std::end(colourPrevalence),
            [](const pair_type &p1, const pair_type &p2) {
              return p1.second < p2.second;
            });
        // now check which it is and set the terrain type
        if (pr->first == namedColours.at("jungle"))
          gameProv.terrainType = "jungle";
        else if (pr->first == namedColours.at("forest"))
          gameProv.terrainType = "forest";
        else if (pr->first == namedColours.at("hills"))
          gameProv.terrainType = "hills";
        else if (pr->first == namedColours.at("mountains") ||
                 pr->first == namedColours.at("peaks"))
          gameProv.terrainType = "mountain";
        else if (pr->first == namedColours.at("grassland") ||
                 pr->first == namedColours.at("savannah"))
          gameProv.terrainType = "plains";
        else if (pr->first == namedColours.at("desert"))
          gameProv.terrainType = "desert";
        else
          gameProv.terrainType = "plains";
        gameProvinces[gameProv.ID].terrainType = gameProv.terrainType;
        for (auto pix : gameProv.baseProvince->pixels) {
          if (pr->first == namedColours.at("jungle"))
            typeMap.setColourAtIndex(pix, Fwg::Gfx::Colour{255, 255, 0});
          else if (pr->first == namedColours.at("forest"))
            typeMap.setColourAtIndex(pix, Fwg::Gfx::Colour{0, 255, 0});
          else if (pr->first == namedColours.at("hills"))
            typeMap.setColourAtIndex(pix, Fwg::Gfx::Colour{128, 128, 128});
          else if (pr->first == namedColours.at("mountains") ||
                   pr->first == namedColours.at("peaks"))
            typeMap.setColourAtIndex(pix, Fwg::Gfx::Colour{255, 255, 255});
          else if (pr->first == namedColours.at("grassland") ||
                   pr->first == namedColours.at("savannah"))
            typeMap.setColourAtIndex(pix, Fwg::Gfx::Colour{0, 255, 128});
          else if (pr->first == namedColours.at("desert"))
            typeMap.setColourAtIndex(pix, Fwg::Gfx::Colour{0, 255, 255});
          else
            typeMap.setColourAtIndex(pix, Fwg::Gfx::Colour{255, 0, 0});
        }
      }
  Bitmap::SaveBMPToFile(typeMap, "Maps/typeMap.bmp");
}

Region &Generator::findStartRegion() {
  std::vector<Region> freeRegions;
  for (const auto &gameRegion : gameRegions)
    if (!gameRegion.assigned && !gameRegion.sea)
      freeRegions.push_back(gameRegion);

  if (freeRegions.size() == 0)
    return gameRegions[0];

  const auto &startRegion = Fwg::Utils::selectRandom(freeRegions);
  return gameRegions[startRegion.ID];
}

// generate countries according to given ruleset for each game
// TODO: rulesets, e.g. naming schemes? tags? country size?
void Generator::generateCountries(int numCountries,
                                  const std::string &gamePath) {
  auto &config = Fwg::Env::Instance();
  this->numCountries = numCountries;
  Logging::logLine("Generating Countries");
  // load tags from hoi4 that are used by the base game
  // do not use those to avoid conflicts
  const auto forbiddenTags = ResourceLoading::loadForbiddenTags(gamePath);
  for (const auto &tag : forbiddenTags)
    tags.insert(tag);

  for (int i = 0; i < numCountries; i++) {
    auto name{nG.generateName()};
    PdoxCountry pdoxC(nG.generateTag(name, tags), i, name,
                      nG.generateAdjective(name), Gfx::Flag(82, 52));
    // randomly set development of countries
    pdoxC.developmentFactor = Fwg::RandNum::getRandomDouble(0.1, 1.0);
    countries.emplace(pdoxC.tag, pdoxC);
  }
  for (auto &pdoxCountry : countries) {
    auto startRegion(findStartRegion());
    if (startRegion.assigned || startRegion.sea)
      continue;
    pdoxCountry.second.assignRegions(6, gameRegions, startRegion,
                                     gameProvinces);
  }
  for (auto &gameRegion : gameRegions) {
    if (!gameRegion.sea && !gameRegion.assigned) {
      auto &gR = Fwg::Utils::getNearestAssignedLand(
          gameRegions, gameRegion, config.width, config.height);
      countries.at(gR.owner).addRegion(gameRegion, gameRegions, gameProvinces);
    }
  }
}

void Generator::evaluateNeighbours() {
  Logging::logLine("Evaluating Country Neighbours");
  for (auto &c : countries)
    for (const auto &gR : c.second.ownedRegions)
      for (const auto &neighbourRegion : gameRegions[gR].neighbours)
        if (gameRegions[neighbourRegion].owner != c.first)
          c.second.neighbours.insert(gameRegions[neighbourRegion].owner);
}

void Generator::dumpDebugCountrymap(const std::string &path) {
  Logging::logLine("Mapping Continents");
  auto &config = Fwg::Env::Instance();
  Bitmap countryBMP(config.width, config.height, 24);
  for (const auto &country : countries)
    for (const auto &region : country.second.ownedRegions)
      for (const auto &prov : gameRegions[region].provinces)
        for (const auto &pix : prov->pixels)
          countryBMP.setColourAtIndex(pix, country.second.colour);

  Bitmap::bufferBitmap("countries", countryBMP);
  Bitmap::SaveBMPToFile(countryBMP, (path).c_str());
}
} // namespace Scenario