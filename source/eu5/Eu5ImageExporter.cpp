#include "eu5/Eu5ImageExporter.h"
namespace Rpx::Gfx::Eu5 {
using namespace Arda::Gfx::Textures;
using namespace Fwg;
using namespace Fwg::Gfx;

ImageExporter::ImageExporter() {}

ImageExporter::ImageExporter(const std::string &gamePath,
                             const std::string &gameTag)
    : Rpx::Gfx::ImageExporter(gamePath, gameTag) {}

ImageExporter::~ImageExporter() {}

std::vector<std::vector<float>>
splitInto16Tiles(const std::vector<float> &imageData, int width, int height) {
  const int tilesX = 4;
  const int tilesY = 4;

  int tileW = width / tilesX;
  int tileH = height / tilesY;

  std::vector<std::vector<float>> tiles(tilesX * tilesY);

  // Reverse tile rows (top <-> bottom)
  for (int ty = tilesY - 1; ty >= 0; --ty) {
    for (int tx = 0; tx < tilesX; ++tx) {

      std::vector<float> &tile = tiles[(tilesY - 1 - ty) + tx * tilesY];
      tile.reserve(tileW * tileH);

      for (int y = 0; y < tileH; ++y) {
        int srcY = ty * tileH + y;

        for (int x = 0; x < tileW; ++x) {
          int srcX = tx * tileW + x;
          tile.push_back(imageData[srcY * width + srcX]);
        }
      }
    }
  }

  return tiles;
}

Fwg::Gfx::Bitmap ImageExporter::dumpHeightmap(
    const std::vector<float> &heightMap, const std::string &path,
    const std::string &colourMapKey, int exportWidth, int exportHeight) const {
  auto scaledHeightmap = Fwg::Utils::scaleBufferWithInterpolation(
      heightMap, Fwg::Cfg::Values().width, Fwg::Cfg::Values().height,
      exportWidth, exportHeight);
  // Fwg::Gfx::Png::save(scaledHeightmap, path);
  // rasterize this image into 16 different images , based on the heightmap,
  // presenting 16 different chunks of the heightmap
  auto tiles = splitInto16Tiles(scaledHeightmap, exportWidth, exportHeight);
  int i = 0;
  for (auto &tile : tiles) {
    i++;
    //Fwg::Gfx::Bitmap tileBitmap(exportWidth / 4, exportHeight / 4, 24,
    //                            std::move(tile));
    std::string num = std::to_string(i);
    auto name = "heightmap_" + num + "_16";
    auto hmapPath = path + "//" + name + "//" + name + "_height.png";
    Fwg::Utils::Logging::logLine("Saving heightmap tile: " + hmapPath);
    Fwg::Gfx::Png::save(tile, exportWidth/4, exportHeight/4, hmapPath, true, LCT_GREY, 16);
  }

  return Fwg::Gfx::Bitmap(0, 0, 24);
}

Fwg::Gfx::Bitmap ImageExporter::dumpDecalMasks(
    const Fwg::Terrain::TerrainData &terrainData,
    const Fwg::Climate::ClimateData &climateData, const std::string &path,
    const std::string &colourMapKey, int exportWidth, int exportHeight) const {
  auto imageHumidity = climateData.humidities;
  Fwg::Utils::normalizeVector(imageHumidity, 0.0f, 0.0f);
  auto scaledHeightmap = Fwg::Utils::scaleBufferWithInterpolation(
      imageHumidity, Fwg::Cfg::Values().width, Fwg::Cfg::Values().height,
      exportWidth, exportHeight);

  // rasterize this image into 16 different images , based on the heightmap,
  // presenting 16 different chunks of the heightmap
  auto tiles = splitInto16Tiles(scaledHeightmap, exportWidth, exportHeight);
  int i = 0;
  for (auto &tile : tiles) {
    i++;
    // for now write no ground textures, keep it blank
    Fwg::Gfx::Bitmap tileBitmap(exportWidth / 4, exportHeight / 4, 24,
                                std::move(tile));
    std::string num = std::to_string(i);
    auto folderName = "heightmap_" + num + "_16";
    for (int x = 0; x < 16; x++) {
      auto name = folderName + "_mask_" + std::format("{:02}", x) + ".png";
      auto hmapPath = path + "//" + folderName + "//" + name;
      Fwg::Utils::Logging::logLine("Saving mask tile: " + hmapPath);
      // we want grayscale 8bit non-linear integer
      Fwg::Gfx::Png::save(tileBitmap, hmapPath, true, LCT_GREY);
    }
    Fwg::Utils::Logging::logLine("Saving heightmap bitmask tile: " +
                                 (path + "//" + folderName + "//heightmap_" +
                                  std::to_string(i) + "_16_bitmask.png"));
    tileBitmap.setColourAtIndex(0, 255);
    // also write the heightmap bitmask
    Fwg::Gfx::Png::save(tileBitmap,
                        path + "//" + folderName + "//heightmap_" +
                            std::to_string(i) + "_16_bitmask.png",
                        true, LCT_GREY);
  }

  return Fwg::Gfx::Bitmap(0, 0, 24);
}

Fwg::Gfx::Bitmap ImageExporter::dumpTerrainMasks(
    const Fwg::Terrain::TerrainData &terrainData,
    const Fwg::Climate::ClimateData &climateData, const std::string &path,
    const std::string &colourMapKey, int exportWidth, int exportHeight) const {

  std::map<int, std::string> nameMapping{
      {0, "asia_savannah_base_mask"},
      {1, "asian_farms_mask"},
      {2, "conifer_forest_mask"},
      {3, "continental_grass_dirt_mask"},
      {4, "continental_hills_mask"},
      {5, "coral_green_mask"},
      {6, "coral_mask"},
      {7, "coral_variation_mask"},
      {8, "dirt_01_mask"},
      {9, "elephant_mask"},
      {10, "estepa_base_01_mask"},
      {11, "estepa_variation_01_mask"},
      {12, "farms_01_mask"},
      {13, "farms_02_mask"},
      {14, "forest_01_mask"},
      {15, "hills_base_01_mask"},
      {16, "impassable_desert_canyon_mask"},
      {17, "impassable_desert_variation_01_mask"},
      {18, "impassable_desert_variation_02_mask"},
      {19, "iron_mask"},
      {20, "jungle_variation_soil_01_mask"},
      {21, "marsh_base_01_mask"},
      {22, "marsh_variation_02_mask"},
      {23, "mask_devastation_mud_01"},
      {24, "med_drylands_02_mask"},
      {25, "med_drylands_variation_mask"},
      {26, "mountain_artic_variation_01_mask"},
      {27, "mountain_med_variation_mask"},
      {28, "mountains_gravel_mask"},
      {29, "mountains_variation_dirt_02_mask"},
      {30, "mountains_variation_rock_01_mask"},
      {31, "mud_base_01_mask"},
      {32, "plains_01_mask"},
      {33, "plains_02_mask"},
      {34, "plains_mediterranean_mask"},
      {35, "plains_variation_grass_01_mask"},
      {36, "plains_variation_grass_02_mask"},
      {37, "plains_variation_grass_03_mask"},
      {38, "rock_01_mask"},
      {39, "sand_01_mask"},
      {40, "sand_02_mask"},
      {41, "sand_dunes_01_mask"},
      {42, "sand_dunes_02_mask"},
      {43, "savannah_base_01_mask"},
      {44, "savannah_variation_grass_01_mask"},
      {45, "snow_mask"},
      {46, "tundra_base_01_mask"},
      {47, "tundra_variation_grass_01_mask"},
      {48, "tundra_variation_rock_02_mask"},
      {49, "water_mask"},
  };

  auto imageHumidity = climateData.humidities;
  Fwg::Utils::normalizeVector(imageHumidity, 0.0f, 255.0f);
  for (int i = 0; i < 50; i++) {
    FastNoiseLite noiseGenerator;
    noiseGenerator.SetSeed(i);
    noiseGenerator.SetFrequency(0.05);
    noiseGenerator.SetFractalOctaves(11);
    noiseGenerator.SetFractalGain(0.7f);
    noiseGenerator.SetNoiseType(
        FastNoiseLite::NoiseType_ValueCubic); // Set the desired noise type
    noiseGenerator.SetFractalType(FastNoiseLite::FractalType_FBm);
    // randomness for trees
    auto treeNoiseMap =
        Noise::genNoise(noiseGenerator, Fwg::Cfg::Values().width,
                        Fwg::Cfg::Values().height, 0.0, 0.0, 0, 255, 0.0);
    auto scaledHeightmap = Fwg::Utils::scaleBufferWithInterpolation(
        treeNoiseMap, Fwg::Cfg::Values().width, Fwg::Cfg::Values().height,
        exportWidth, exportHeight);
    Fwg::Gfx::Png::save(
        Fwg::Gfx::Bitmap(exportWidth, exportHeight, 24, scaledHeightmap),
        path + "//" + nameMapping.at(i) + ".png", true, LCT_GREY);
  }

  return Fwg::Gfx::Bitmap(0, 0, 24);
}

void ImageExporter::Eu5ColourMaps(
    const Fwg::Terrain::TerrainData &terrainData,
    const Fwg::Climate::ClimateData &climateData,
    const Arda::Civilization::CivilizationLayer &civLayer,
    const std::string &path, int exportWidth, int exportHeight) {
  Fwg::Utils::Logging::logLine("Eu5 Format Converter: Writing colour maps");

  auto &config = Cfg::Values();
  auto altitude = terrainData.detailedHeightMap;
  auto scaledHeightmap = Fwg::Utils::scaleBufferWithInterpolation(
      altitude, Fwg::Cfg::Values().width, Fwg::Cfg::Values().height,
      exportWidth, exportHeight);
  int factor = 1;

  std::vector<uint8_t> pixels(exportWidth * exportHeight * 4, 0);
  Utils::Logging::logLine(
      "ImageExporter::Writing watercolor_rgb_waterspec_a to ", path);
  using namespace DirectX;

  for (auto h = 0; h < exportHeight; h++) {
    for (auto w = 0; w < exportWidth; w++) {
      auto referenceIndex = h * exportWidth + w;
      double depth = (double)scaledHeightmap[referenceIndex] /
                     (double)Cfg::Values().seaLevel;
      auto imageIndex =
          exportHeight * exportWidth - (h * exportWidth + (exportWidth - w));
      imageIndex *= 4;
      if (depth < 1.0) {
        writeBufferPixels(
            pixels, imageIndex,
            Fwg::Gfx::Colour(16.0 * depth, 24.0 * depth, 49.0 * depth), 255);
      } else {
        writeBufferPixels(pixels, imageIndex, Fwg::Gfx::Colour(50, 100, 100),
                          255);
      }
    }
  }
  Arda::Gfx::Textures::writeMipMapDDS(
      exportWidth, exportHeight, pixels, DXGI_FORMAT_B8G8R8A8_UNORM,
      path + "//water//watercolor_rgb_waterspec_a.dds", true);
  // std::fill(pixels.begin(), pixels.end(), 0);
  // Arda::Gfx::Textures::writeMipMapDDS(exportWidth / 4, exportHeight / 4,
  // pixels,
  //                                     DXGI_FORMAT_B8G8R8A8_UNORM,
  //                                     path + "//water//foam_map.dds");
  // Arda::Gfx::Textures::writeMipMapDDS(exportWidth / 8, exportHeight / 8,
  // pixels,
  //                                     DXGI_FORMAT_B8G8R8A8_UNORM,
  //                                     path + "//water//flowmap.dds");
}

void ImageExporter::mapObjectMasks(
    const Fwg::Terrain::TerrainData &terrainData,
    const Fwg::Climate::ClimateData &climateData,
    const Arda::Civilization::CivilizationLayer &civLayer,
    const std::string &path, int exportWidth, int exportHeight) {
  auto width = Cfg::Values().width;
  auto height = Cfg::Values().height;

  std::vector<std::string> emptyMasks = {
      "decals_mask",
      "dirt_mask",
      "grassmesh_mask",
      "reeds_mask",
      "rock_mask",
      "vegetation_forest_japan_mask",
      "vegetation_woods_japan_mask",
  };
  std::shared_ptr<Fwg::Gfx::Bitmap> denseJungleMask =
      std::make_shared<Fwg::Gfx::Bitmap>(width, height, 24);
  std::shared_ptr<Fwg::Gfx::Bitmap> jungleMask =
      std::make_shared<Fwg::Gfx::Bitmap>(width, height, 24);

  std::shared_ptr<Fwg::Gfx::Bitmap> pineMask =
      std::make_shared<Fwg::Gfx::Bitmap>(width, height, 24);

  std::shared_ptr<Fwg::Gfx::Bitmap> forestContinentalMask =
      std::make_shared<Fwg::Gfx::Bitmap>(width, height, 24);
  std::shared_ptr<Fwg::Gfx::Bitmap> woodsContinentalMask =
      std::make_shared<Fwg::Gfx::Bitmap>(width, height, 24);

  for (int i = 0; i < climateData.treeCoverage.size(); i++) {
    auto treeCoverType = climateData.treeCoverage[i];
    auto treeDensity = static_cast<float>(climateData.treeDensity[i]) / 255.0f;
    switch (treeCoverType) {
    case Fwg::Climate::Detail::TreeTypeIndex::TROPICALMOIST: {
      denseJungleMask->setColourAtIndex(i, 255.0f * treeDensity);
      break;
    }
    case Fwg::Climate::Detail::TreeTypeIndex::TROPICALDRY: {
      jungleMask->setColourAtIndex(i, 255.0f * treeDensity);
      break;
    }
    case Fwg::Climate::Detail::TreeTypeIndex::BOREAL: {
      pineMask->setColourAtIndex(i, 255.0f * treeDensity);
      break;
    }
    case Fwg::Climate::Detail::TreeTypeIndex::TEMPERATEMIXED: {
      if (treeDensity > 0.5f) {
        forestContinentalMask->setColourAtIndex(i, 255.0f * treeDensity);
      } else {
        woodsContinentalMask->setColourAtIndex(i, 255.0f * treeDensity);
      }
      break;
    }
    case Fwg::Climate::Detail::TreeTypeIndex::TEMPERATENEEDLE: {
      pineMask->setColourAtIndex(i, 255.0f * treeDensity);
      break;
    }
    default:
      break;
    }
  }
  Fwg::Gfx::Png::save(
      Fwg::Gfx::Bmp::scale(*denseJungleMask, exportWidth, exportHeight, false),
      path + "//jungle_dense_mask.png", false, LCT_GREY);
  Fwg::Gfx::Png::save(
      Fwg::Gfx::Bmp::scale(*jungleMask, exportWidth, exportHeight, false),
      path + "//jungle_mask.png", false, LCT_GREY);
  Fwg::Gfx::Png::save(
      Fwg::Gfx::Bmp::scale(*pineMask, exportWidth, exportHeight, false),
      path + "//pine_mask.png", false, LCT_GREY);
  Fwg::Gfx::Png::save(Fwg::Gfx::Bmp::scale(*forestContinentalMask, exportWidth,
                                           exportHeight, false),
                      path + "//vegetation_forest_continental_oceanic_mask.png",
                      false, LCT_GREY);
  Fwg::Gfx::Png::save(Fwg::Gfx::Bmp::scale(*woodsContinentalMask, exportWidth,
                                           exportHeight, false),
                      path + "//vegetation_woods_continental_oceanic_mask.png",
                      false, LCT_GREY);

  auto blankBitmap = Fwg::Gfx::Bitmap(exportWidth, exportHeight, 24);
  blankBitmap.setColourAtIndex(0, 255);
  for (auto &maskName : emptyMasks) {
    Fwg::Gfx::Png::save(blankBitmap,
        path + "//" + maskName + ".png", true, LCT_GREY);
  }
}

void ImageExporter::writeLocations(
    const Fwg::Gfx::Bitmap &provinceMap,
    const std::vector<std::shared_ptr<Arda::ArdaProvince>> &provinces,
    const std::vector<std::shared_ptr<Arda::ArdaRegion>> &regions,
    const std::vector<std::shared_ptr<Arda::ArdaContinent>> &continents,
    const Arda::Civilization::CivilizationLayer &civLayer,
    const std::string &path, int exportWidth, int exportHeight) const {

  Fwg::Utils::Logging::logLine("Eu5 Format Converter: Writing locations map");

  // flatland, hills, mountain_wasteland, mountains, wetlands, lakes,
  // high_lakes, plateau, narrows, coastal_ocean, inland_sea, ocean, deep_ocean,
  // dune_wasteland, mesa_wasteland
  std::map<Fwg::Terrain::ElevationTypeIndex, std::string> elevationMapping = {
      {Fwg::Terrain::ElevationTypeIndex::OCEAN, "ocean"},
      {Fwg::Terrain::ElevationTypeIndex::DEEPOCEAN, "deep_ocean"},
      {Fwg::Terrain::ElevationTypeIndex::PLAINS, "flatland"},
      {Fwg::Terrain::ElevationTypeIndex::LOWHILLS, "hills"},
      {Fwg::Terrain::ElevationTypeIndex::HILLS, "hills"},
      {Fwg::Terrain::ElevationTypeIndex::MOUNTAINS, "mountains"},
      {Fwg::Terrain::ElevationTypeIndex::PEAKS, "mountains"},
      {Fwg::Terrain::ElevationTypeIndex::STEEPPEAKS, "mountains"},
      {Fwg::Terrain::ElevationTypeIndex::CLIFF, "mountains"},
      {Fwg::Terrain::ElevationTypeIndex::HIGHLANDS, "plateau"},
      {Fwg::Terrain::ElevationTypeIndex::VALLEY, "hills"},
      {Fwg::Terrain::ElevationTypeIndex::LAKE, "lakes"},
  };
  enum class ClimateTypeIndex : unsigned char {
    TROPICSRAINFOREST,
    TROPICSMONSOON,
    TROPICSSAVANNA,
    DESERT,
    COLDDESERT,
    HOTSEMIARID,
    COLDSEMIARID,
    TEMPERATEHOT,
    TEMPERATEWARM,
    TEMPERATECOLD,
    CONTINENTALHOT,
    CONTINENTALWARM,
    CONTINENTALCOLD,
    POLARTUNDRA,
    POLARARCTIC,
    SNOW,
    WATER
  };
  // continental, arctic, oceanic, mediterranean, subtropical, tropical,
  // cold_arid, arid,
  std::map<Fwg::Climate::Detail::ClimateTypeIndex, std::string> climateMapping =
      {
          {Fwg::Climate::Detail::ClimateTypeIndex::TROPICSRAINFOREST,
           "tropical"},
          {Fwg::Climate::Detail::ClimateTypeIndex::TROPICSMONSOON, "tropical"},
          {Fwg::Climate::Detail::ClimateTypeIndex::TROPICSSAVANNA,
           "subtropical"},
          {Fwg::Climate::Detail::ClimateTypeIndex::DESERT, "arid"},
          {Fwg::Climate::Detail::ClimateTypeIndex::COLDDESERT, "cold_arid"},
          {Fwg::Climate::Detail::ClimateTypeIndex::HOTSEMIARID, "arid"},
          {Fwg::Climate::Detail::ClimateTypeIndex::COLDSEMIARID, "cold_arid"},
          {Fwg::Climate::Detail::ClimateTypeIndex::TEMPERATEHOT,
           "mediterranean"},
          {Fwg::Climate::Detail::ClimateTypeIndex::TEMPERATEWARM,
           "mediterranean"},
          {Fwg::Climate::Detail::ClimateTypeIndex::TEMPERATECOLD,
           "continental"},
          {Fwg::Climate::Detail::ClimateTypeIndex::CONTINENTALHOT,
           "continental"},
          {Fwg::Climate::Detail::ClimateTypeIndex::CONTINENTALWARM,
           "continental"},
          {Fwg::Climate::Detail::ClimateTypeIndex::CONTINENTALCOLD,
           "continental"},
          {Fwg::Climate::Detail::ClimateTypeIndex::POLARTUNDRA, "continental"},
          {Fwg::Climate::Detail::ClimateTypeIndex::POLARARCTIC, "arctic"},
          {Fwg::Climate::Detail::ClimateTypeIndex::SNOW, "arctic"},
          {Fwg::Climate::Detail::ClimateTypeIndex::WATER, "ocean"},
      };

  auto scaledProvinces =
      Fwg::Gfx::Bmp::scale(provinceMap, exportWidth, exportHeight, false);
  Fwg::Gfx::Png::save(scaledProvinces, path + "//locations.png", true);

  // vegetation: grassland, farmland, woods, forest, sparse, desert, jungle,

  // now lets write the hex codes of all the locations to the
  // named_locations/00_default.txt file
  std::string fileData = "";
  std::string locationTemplates = "";
  int id = 0;
  std::string lakeText = "{ topography = lakes climate = continental}";
  std::string landText =
      "{ topography = templateTopography vegetation = sparse climate = "
      "templateClimate "
      "religion = catholic culture = swedish raw_material = lumber}";
  std::string coastalText =
      "{ topography = templateTopography vegetation = sparse climate = "
      "templateClimate "
      "religion = catholic culture = swedish raw_material = fish "
      "natural_harbor_suitability = 0.00}";
  std::string oceanText = "{ topography = coastal_ocean climate = arctic}";

  std::string oceanZones = "";
  std::string lakeZones = "";

  for (auto &province : provinces) {
    auto topography = province->dominantElevationType;
    auto climateType = province->climateType;
    auto provName = "prov_" + std::to_string(province->ID);
    fileData.append(provName + " = " + province->toHexString(false, false) +
                    "\n");
    locationTemplates.append(provName + " = ");
    if (province->isSea()) {
      locationTemplates.append(oceanText + "\n");
      oceanZones.append(provName + "\n");
    } else if (province->isLake()) {
      locationTemplates.append(lakeText + "\n");
      lakeZones.append(provName + "\n");
    } else if (province->coastal) {
      locationTemplates.append(coastalText + "\n");
    } else {
      locationTemplates.append(landText + "\n");
    }
    Fwg::Parsing::replaceOccurences(locationTemplates, "templateTopography",
                                    elevationMapping[topography]);
    Fwg::Parsing::replaceOccurences(locationTemplates, "templateClimate",
                                    climateMapping[climateType]);
  }

  Fwg::Parsing::writeFile(path + "//named_locations/00_default.txt", fileData,
                          true);

  Fwg::Parsing::writeFile(path + "//definitions.txt", "");

  Fwg::Parsing::writeFile(path + "//location_templates.txt", locationTemplates);

  // lets try port
  // for every coastal province, we write a port entry, which is any
  // neighbouring province that is sea
  std::string oceanPortTemplate = "LandProvince;SeaZone;x;y;\n";
  for (auto &province : provinces) {
    if (!province->coastal) {
      continue;
    }
    auto coastalProvName = "prov_" + std::to_string(province->ID);
    std::string targetName = "";
    for (auto &neighbourRelation : province->neighbourRelations) {
      if (neighbourRelation->neighbour->isSea()) {
        // we have a coastal province with a sea neighbour, write port entry
        targetName = "prov_" + std::to_string(neighbourRelation->neighbour->ID);
      }
    }
    auto x = province->position.widthCenter;
    auto y = province->position.heightCenter;
    auto assembledLine = coastalProvName + ";" + targetName + ";" +
                         std::to_string(x) + ";" + std::to_string(y) + ";\n";
    oceanPortTemplate.append(assembledLine);
  }

  Fwg::Parsing::writeFile(path + "//ports.csv", oceanPortTemplate);

  auto defaultMapTemplate = Fwg::Parsing::readFile(Cfg::Values().resourcePath +
                                                   "eu5/map_data/default.map");
  Fwg::Parsing::replaceOccurences(defaultMapTemplate, "templateSeaZones",
                                  oceanZones);
  Fwg::Parsing::replaceOccurences(defaultMapTemplate, "templateLakes",
                                  lakeZones);
  Fwg::Parsing::writeFile(path + "//default.map", defaultMapTemplate);
  std::string definitionFileData = "";

  for (auto &continent : continents) {
    std::string continentData = std::to_string(continent->ID) + " = {\n";
    continentData.append("\t" + std::to_string(continent->ID) + "_sub = {\n");
    continentData.append("\t\t" + std::to_string(continent->ID) + "_reg = {\n");
    continentData.append("\t\t\t" + std::to_string(continent->ID) +
                         "_area = {\n");
    for (auto &region : continent->ardaRegions) {
      continentData.append("\t\t\t\t" + std::to_string(region->ID) +
                           "_province = {\n");
      for (auto &province : region->ardaProvinces) {
        continentData.append("\t\t\t\t\tprov_" + std::to_string(province->ID) +
                             "\n");
      }
      continentData.append("\t\t\t\t}\n");
    }
    continentData.append("\t\t\t}\n");
    continentData.append("\t\t}\n");
    continentData.append("\t}\n");
    continentData.append("}\n");
    definitionFileData.append(continentData);
  }

  std::string continentData = "water_1 = {\n";
  continentData.append("\twater_1_sub = {\n");
  continentData.append("\t\twater_1_reg = {\n");
  continentData.append("\t\twater_1_area = {\n");
  for (auto &region : regions) {
    if (!region->isSea()) {
      continue;
    }
    continentData.append("\t\t\t\t" + std::to_string(region->ID) +
                         "_province = {\n");
    for (auto &province : region->ardaProvinces) {
      continentData.append("\t\t\t\t\tprov_" + std::to_string(province->ID) +
                           "\n");
    }
    continentData.append("\t\t\t\t}\n");
  }
  continentData.append("\t\t\t}\n");
  continentData.append("\t\t}\n");
  continentData.append("\t}\n");
  continentData.append("}\n");
  definitionFileData.append(continentData);

  Fwg::Parsing::writeFile(path + "//definitions.txt", definitionFileData);

  Fwg::Parsing::writeFile(
      path + "//adjacencies.csv",
      "From;To;Type;Through;start_x;start_y;stop_x;stop_y;Comment");
}

} // namespace Rpx::Gfx::Eu5