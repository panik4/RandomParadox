#include "io/ImageExporter.h"
namespace Rpx::Gfx {
using namespace Fwg;
using namespace Fwg::Gfx;

const std::map<std::string, std::map<Gfx::Colour, int>>
    ImageExporter::colourMaps{
        {"terrainHoi4",
         {{Cfg::Values().colours["rockyHills"], 2},
          {Cfg::Values().colours["snowyHills"], 16},
          {Cfg::Values().colours["rockyMountains"], 6},
          {Cfg::Values().colours["snowyMountains"], 16},
          {Cfg::Values().colours["rockyPeaks"], 11},
          {Cfg::Values().colours["snowyPeaks"], 16},
          {Cfg::Values().colours["grassland"], 0},
          {Cfg::Values().colours["grasslandHills"], 17},
          {Cfg::Values().colours["grasslandMountains"], 20},
          {Cfg::Values().colours["desert"], 3},
          {Cfg::Values().colours["desertHills"], 8},
          {Cfg::Values().colours["desertMountains"], 10},
          {Cfg::Values().colours["forest"], 4},
          {Cfg::Values().colours["forestHills"], 1},
          {Cfg::Values().colours["forestMountains"], 4},
          {Cfg::Values().colours["savanna"], 0},
          {Cfg::Values().colours["drysavanna"], 12},
          {Cfg::Values().colours["jungle"], 21},
          {Cfg::Values().colours["tundra"], 19},
          {Cfg::Values().colours["ice"], 2},
          {Cfg::Values().colours["marsh"], 9},
          {Cfg::Values().colours["urban"], 13},
          {Cfg::Values().colours["farm"], 5},
          {Cfg::Values().colours["sea"], 15}}},
        {"riversHoi4",
         {{Cfg::Values().colours["land"], 255},
          {Cfg::Values().colours["river"], 3},
          {Cfg::Values().colours["river"] * 0.9, 3},
          {Cfg::Values().colours["river"] * 0.8, 6},
          {Cfg::Values().colours["river"] * 0.7, 6},
          {Cfg::Values().colours["river"] * 0.6, 10},
          {Cfg::Values().colours["river"] * 0.5, 11},
          {Cfg::Values().colours["river"] * 0.3, 11},
          {Cfg::Values().colours["river"] * 0.2, 11},
          {Cfg::Values().colours["river"] * 0.1, 11},
          {Cfg::Values().colours["sea"], 254},
          {Cfg::Values().colours["riverStart"], 0},
          {Cfg::Values().colours["riverStartTributary"], 3},
          {Cfg::Values().colours["riverEnd"], 1}}},
        {"treesHoi4",
         {{Cfg::Values().colours["rockyHills"], 5},
          {Cfg::Values().colours["snowyHills"], 5},
          {Cfg::Values().colours["rockyMountains"], 5},
          {Cfg::Values().colours["snowyMountains"], 5},
          {Cfg::Values().colours["rockyPeaks"], 5},
          {Cfg::Values().colours["snowyPeaks"], 5},
          {Cfg::Values().colours["grassland"], 5},
          {Cfg::Values().colours["grasslandHills"], 6},
          {Cfg::Values().colours["grasslandMountains"], 6},
          {Cfg::Values().colours["desert"], 2},
          {Cfg::Values().colours["desertHills"], 2},
          {Cfg::Values().colours["desertMountains"], 2},
          {Cfg::Values().colours["forest"], 6},
          {Cfg::Values().colours["forestHills"], 6},
          {Cfg::Values().colours["forestMountains"], 6},
          {Cfg::Values().colours["savanna"], 3},
          {Cfg::Values().colours["drysavanna"], 2},
          {Cfg::Values().colours["jungle"], 28},
          {Cfg::Values().colours["tundra"], 5},
          {Cfg::Values().colours["ice"], 5},
          {Cfg::Values().colours["marsh"], 5},
          {Cfg::Values().colours["urban"], 5},
          {Cfg::Values().colours["farm"], 5},
          {Cfg::Values().colours["empty"], 0},
          {Cfg::Values().colours["sea"], 0}}},
        {"riversEu4",
         {{Cfg::Values().colours["land"], 255},
          {Cfg::Values().colours["river"], 3},
          {Cfg::Values().colours["river"] * 0.9, 3},
          {Cfg::Values().colours["river"] * 0.8, 5},
          {Cfg::Values().colours["river"] * 0.7, 7},
          {Cfg::Values().colours["river"] * 0.6, 9},
          {Cfg::Values().colours["river"] * 0.5, 10},
          {Cfg::Values().colours["river"] * 0.4, 11},
          {Cfg::Values().colours["sea"], 254},
          {Cfg::Values().colours["riverStart"], 0},
          {Cfg::Values().colours["riverStartTributary"], 3},
          {Cfg::Values().colours["riverEnd"], 1}}},
        {"treesEu4",
         {{Cfg::Values().colours["rockyHills"], 0},
          {Cfg::Values().colours["snowyHills"], 0},
          {Cfg::Values().colours["rockyMountains"], 0},
          {Cfg::Values().colours["snowyMountains"], 0},
          {Cfg::Values().colours["rockyPeaks"], 0},
          {Cfg::Values().colours["snowyPeaks"], 0},
          {Cfg::Values().colours["grassland"], 0},
          {Cfg::Values().colours["grasslandHills"], 0},
          {Cfg::Values().colours["grasslandMountains"], 0},
          {Cfg::Values().colours["desert"], 0},
          {Cfg::Values().colours["desertHills"], 0},
          {Cfg::Values().colours["desertMountains"], 0},
          {Cfg::Values().colours["forest"], 6},
          {Cfg::Values().colours["forestHills"], 6},
          {Cfg::Values().colours["forestMountains"], 6},
          {Cfg::Values().colours["savanna"], 0},
          {Cfg::Values().colours["drysavanna"], 0},
          {Cfg::Values().colours["jungle"], 28},
          {Cfg::Values().colours["tundra"], 0},
          {Cfg::Values().colours["ice"], 0},
          {Cfg::Values().colours["marsh"], 0},
          {Cfg::Values().colours["urban"], 0},
          {Cfg::Values().colours["farm"], 0},
          {Cfg::Values().colours["empty"], 0},
          {Cfg::Values().colours["sea"], 0}}},
        {"terrainEu4",
         {{Cfg::Values().colours["rockyHills"], 1},
          {Cfg::Values().colours["snowyHills"], 16},
          {Cfg::Values().colours["rockyMountains"], 6},
          {Cfg::Values().colours["snowyMountains"], 16},
          {Cfg::Values().colours["rockyPeaks"], 6},
          {Cfg::Values().colours["snowyPeaks"], 16},
          {Cfg::Values().colours["grassland"], 0},
          {Cfg::Values().colours["grasslandHills"], 0},
          {Cfg::Values().colours["grasslandMountains"], 0},
          {Cfg::Values().colours["desert"], 4},
          {Cfg::Values().colours["desertHills"], 4},
          {Cfg::Values().colours["desertMountains"], 4},
          {Cfg::Values().colours["forest"], 1},
          {Cfg::Values().colours["forestHills"], 1},
          {Cfg::Values().colours["forestMountains"], 1},
          {Cfg::Values().colours["savanna"], 0},
          {Cfg::Values().colours["drysavanna"], 0},
          {Cfg::Values().colours["jungle"], 28},
          {Cfg::Values().colours["tundra"], 0},
          {Cfg::Values().colours["ice"], 16},
          {Cfg::Values().colours["marsh"], 0},
          {Cfg::Values().colours["urban"], 0},
          {Cfg::Values().colours["farm"], 0},
          {Cfg::Values().colours["sea"], 15}}}

    };
using namespace Fwg::Climate::Detail;
const std::map<std::string, std::map<int, int>> ImageExporter::indexMaps{
    {"terrainHoi4",
     {{(int)ClimateTypeIndex::TROPICSRAINFOREST, 21},
      {100 + (int)ClimateTypeIndex::TROPICSRAINFOREST, 22},
      {200 + (int)ClimateTypeIndex::TROPICSRAINFOREST, 27},
      {300 + (int)ClimateTypeIndex::TROPICSRAINFOREST, 27},
      {(int)ClimateTypeIndex::TROPICSMONSOON, 22},
      {100 + (int)ClimateTypeIndex::TROPICSMONSOON, 22},
      {200 + (int)ClimateTypeIndex::TROPICSMONSOON, 27},
      {300 + (int)ClimateTypeIndex::TROPICSMONSOON, 27},
      {(int)ClimateTypeIndex::TROPICSSAVANNA, 5},
      {100 + (int)ClimateTypeIndex::TROPICSSAVANNA, 18},
      {200 + (int)ClimateTypeIndex::TROPICSSAVANNA, 18},
      {300 + (int)ClimateTypeIndex::TROPICSSAVANNA, 27},
      {(int)ClimateTypeIndex::DESERT, 7},
      {100 + (int)ClimateTypeIndex::DESERT, 8},
      {200 + (int)ClimateTypeIndex::DESERT, 10},
      {300 + (int)ClimateTypeIndex::DESERT, 31},
      {(int)ClimateTypeIndex::COLDDESERT, 3},
      {100 + (int)ClimateTypeIndex::COLDDESERT, 8},
      {200 + (int)ClimateTypeIndex::COLDDESERT, 19},
      {300 + (int)ClimateTypeIndex::COLDDESERT, 16},
      {(int)ClimateTypeIndex::HOTSEMIARID, 3},
      {100 + (int)ClimateTypeIndex::HOTSEMIARID, 2},
      {200 + (int)ClimateTypeIndex::HOTSEMIARID, 19},
      {300 + (int)ClimateTypeIndex::HOTSEMIARID, 10},
      {(int)ClimateTypeIndex::COLDSEMIARID, 12},
      {100 + (int)ClimateTypeIndex::COLDSEMIARID, 18},
      {200 + (int)ClimateTypeIndex::COLDSEMIARID, 19},
      {300 + (int)ClimateTypeIndex::COLDSEMIARID, 11},
      {(int)ClimateTypeIndex::TEMPERATEHOT, 0},
      {100 + (int)ClimateTypeIndex::TEMPERATEHOT, 2},
      {200 + (int)ClimateTypeIndex::TEMPERATEHOT, 20},
      {300 + (int)ClimateTypeIndex::TEMPERATEHOT, 16},
      {(int)ClimateTypeIndex::TEMPERATEWARM, 4},
      {100 + (int)ClimateTypeIndex::TEMPERATEWARM, 2},
      {200 + (int)ClimateTypeIndex::TEMPERATEWARM, 20},
      {300 + (int)ClimateTypeIndex::TEMPERATEWARM, 16},
      {(int)ClimateTypeIndex::TEMPERATECOLD, 17},
      {100 + (int)ClimateTypeIndex::TEMPERATECOLD, 2},
      {200 + (int)ClimateTypeIndex::TEMPERATECOLD, 16},
      {300 + (int)ClimateTypeIndex::TEMPERATECOLD, 16},
      {(int)ClimateTypeIndex::CONTINENTALHOT, 1},
      {100 + (int)ClimateTypeIndex::CONTINENTALHOT, 2},
      {200 + (int)ClimateTypeIndex::CONTINENTALHOT, 20},
      {300 + (int)ClimateTypeIndex::CONTINENTALHOT, 16},
      {(int)ClimateTypeIndex::CONTINENTALWARM, 4},
      {100 + (int)ClimateTypeIndex::CONTINENTALWARM, 2},
      {200 + (int)ClimateTypeIndex::CONTINENTALWARM, 20},
      {300 + (int)ClimateTypeIndex::CONTINENTALWARM, 16},
      {(int)ClimateTypeIndex::CONTINENTALCOLD, 1},
      {100 + (int)ClimateTypeIndex::CONTINENTALCOLD, 2},
      {200 + (int)ClimateTypeIndex::CONTINENTALCOLD, 16},
      {300 + (int)ClimateTypeIndex::CONTINENTALCOLD, 16},
      {(int)ClimateTypeIndex::POLARTUNDRA, 0},
      {100 + (int)ClimateTypeIndex::POLARTUNDRA, 18},
      {200 + (int)ClimateTypeIndex::POLARTUNDRA, 16},
      {300 + (int)ClimateTypeIndex::POLARTUNDRA, 16},
      {(int)ClimateTypeIndex::POLARARCTIC, 16},
      {100 + (int)ClimateTypeIndex::POLARARCTIC, 16},
      {200 + (int)ClimateTypeIndex::POLARARCTIC, 16},
      {300 + (int)ClimateTypeIndex::POLARARCTIC, 16},
      {(int)ClimateTypeIndex::SNOW, 16},
      {100 + (int)ClimateTypeIndex::SNOW, 16},
      {200 + (int)ClimateTypeIndex::SNOW, 16},
      {300 + (int)ClimateTypeIndex::SNOW, 16},
      {(int)ClimateTypeIndex::WATER, 15},
      {100 + (int)ClimateTypeIndex::WATER, 15},
      {200 + (int)ClimateTypeIndex::WATER, 15},
      {300 + (int)ClimateTypeIndex::WATER, 15}

     }},
    {"treeterrainHoi4",
     {{(int)TreeTypeIndex::NONE, 0},
      {(int)TreeTypeIndex::BOREAL, 1},
      {(int)TreeTypeIndex::TEMPERATENEEDLE, 1},
      {(int)TreeTypeIndex::TEMPERATEMIXED, 4},
      {(int)TreeTypeIndex::SPARSE, 4},
      {(int)TreeTypeIndex::TROPICALDRY, 22},
      {(int)TreeTypeIndex::TROPICALMOIST, 21}}},
    {"treesHoi4",
     {{(int)TreeTypeIndex::NONE, 0},
      {(int)TreeTypeIndex::BOREAL, 6},
      {(int)TreeTypeIndex::TEMPERATENEEDLE, 6},
      {(int)TreeTypeIndex::TEMPERATEMIXED, 5},
      {(int)TreeTypeIndex::SPARSE, 3},
      {(int)TreeTypeIndex::TROPICALDRY, 28},
      {(int)TreeTypeIndex::TROPICALMOIST, 29}}},
    {"riversHoi4",
     {{(int)0, 0},
      {(int)1, 6},
      {(int)2, 6},
      {(int)TreeTypeIndex::TEMPERATEMIXED, 5},
      {(int)TreeTypeIndex::SPARSE, 3},
      {(int)TreeTypeIndex::TROPICALDRY, 28},
      {(int)TreeTypeIndex::TROPICALMOIST, 29}}},
    {"terrainEu4",
     {{(int)ClimateTypeIndex::TROPICSRAINFOREST, 21},
      {100 + (int)ClimateTypeIndex::TROPICSRAINFOREST, 22},
      {200 + (int)ClimateTypeIndex::TROPICSRAINFOREST, 27},
      {300 + (int)ClimateTypeIndex::TROPICSRAINFOREST, 27},
      {(int)ClimateTypeIndex::TROPICSMONSOON, 22},
      {100 + (int)ClimateTypeIndex::TROPICSMONSOON, 22},
      {200 + (int)ClimateTypeIndex::TROPICSMONSOON, 27},
      {300 + (int)ClimateTypeIndex::TROPICSMONSOON, 27},
      {(int)ClimateTypeIndex::TROPICSSAVANNA, 5},
      {100 + (int)ClimateTypeIndex::TROPICSSAVANNA, 18},
      {200 + (int)ClimateTypeIndex::TROPICSSAVANNA, 18},
      {300 + (int)ClimateTypeIndex::TROPICSSAVANNA, 27},
      {(int)ClimateTypeIndex::DESERT, 7},
      {100 + (int)ClimateTypeIndex::DESERT, 8},
      {200 + (int)ClimateTypeIndex::DESERT, 10},
      {300 + (int)ClimateTypeIndex::DESERT, 31},
      {(int)ClimateTypeIndex::COLDDESERT, 3},
      {100 + (int)ClimateTypeIndex::COLDDESERT, 8},
      {200 + (int)ClimateTypeIndex::COLDDESERT, 19},
      {300 + (int)ClimateTypeIndex::COLDDESERT, 16},
      {(int)ClimateTypeIndex::HOTSEMIARID, 3},
      {100 + (int)ClimateTypeIndex::HOTSEMIARID, 2},
      {200 + (int)ClimateTypeIndex::HOTSEMIARID, 19},
      {300 + (int)ClimateTypeIndex::HOTSEMIARID, 10},
      {(int)ClimateTypeIndex::COLDSEMIARID, 12},
      {100 + (int)ClimateTypeIndex::COLDSEMIARID, 18},
      {200 + (int)ClimateTypeIndex::COLDSEMIARID, 19},
      {300 + (int)ClimateTypeIndex::COLDSEMIARID, 11},
      {(int)ClimateTypeIndex::TEMPERATEHOT, 0},
      {100 + (int)ClimateTypeIndex::TEMPERATEHOT, 2},
      {200 + (int)ClimateTypeIndex::TEMPERATEHOT, 20},
      {300 + (int)ClimateTypeIndex::TEMPERATEHOT, 16},
      {(int)ClimateTypeIndex::TEMPERATEWARM, 4},
      {100 + (int)ClimateTypeIndex::TEMPERATEWARM, 2},
      {200 + (int)ClimateTypeIndex::TEMPERATEWARM, 20},
      {300 + (int)ClimateTypeIndex::TEMPERATEWARM, 16},
      {(int)ClimateTypeIndex::TEMPERATECOLD, 17},
      {100 + (int)ClimateTypeIndex::TEMPERATECOLD, 2},
      {200 + (int)ClimateTypeIndex::TEMPERATECOLD, 16},
      {300 + (int)ClimateTypeIndex::TEMPERATECOLD, 16},
      {(int)ClimateTypeIndex::CONTINENTALHOT, 1},
      {100 + (int)ClimateTypeIndex::CONTINENTALHOT, 2},
      {200 + (int)ClimateTypeIndex::CONTINENTALHOT, 20},
      {300 + (int)ClimateTypeIndex::CONTINENTALHOT, 16},
      {(int)ClimateTypeIndex::CONTINENTALWARM, 4},
      {100 + (int)ClimateTypeIndex::CONTINENTALWARM, 2},
      {200 + (int)ClimateTypeIndex::CONTINENTALWARM, 20},
      {300 + (int)ClimateTypeIndex::CONTINENTALWARM, 16},
      {(int)ClimateTypeIndex::CONTINENTALCOLD, 1},
      {100 + (int)ClimateTypeIndex::CONTINENTALCOLD, 2},
      {200 + (int)ClimateTypeIndex::CONTINENTALCOLD, 16},
      {300 + (int)ClimateTypeIndex::CONTINENTALCOLD, 16},
      {(int)ClimateTypeIndex::POLARTUNDRA, 0},
      {100 + (int)ClimateTypeIndex::POLARTUNDRA, 18},
      {200 + (int)ClimateTypeIndex::POLARTUNDRA, 16},
      {300 + (int)ClimateTypeIndex::POLARTUNDRA, 16},
      {(int)ClimateTypeIndex::POLARARCTIC, 16},
      {100 + (int)ClimateTypeIndex::POLARARCTIC, 16},
      {200 + (int)ClimateTypeIndex::POLARARCTIC, 16},
      {300 + (int)ClimateTypeIndex::POLARARCTIC, 16},
      {(int)ClimateTypeIndex::SNOW, 16},
      {100 + (int)ClimateTypeIndex::SNOW, 16},
      {200 + (int)ClimateTypeIndex::SNOW, 16},
      {300 + (int)ClimateTypeIndex::SNOW, 16},
      {(int)ClimateTypeIndex::WATER, 15},
      {100 + (int)ClimateTypeIndex::WATER, 15},
      {200 + (int)ClimateTypeIndex::WATER, 15},
      {300 + (int)ClimateTypeIndex::WATER, 15}

     }},
    {"treeterrainEu4",
     {{(int)TreeTypeIndex::NONE, 0},
      {(int)TreeTypeIndex::BOREAL, 1},
      {(int)TreeTypeIndex::TEMPERATENEEDLE, 1},
      {(int)TreeTypeIndex::TEMPERATEMIXED, 4},
      {(int)TreeTypeIndex::SPARSE, 4},
      {(int)TreeTypeIndex::TROPICALDRY, 22},
      {(int)TreeTypeIndex::TROPICALMOIST, 21}}},
    {"treesEu4",
     {{(int)TreeTypeIndex::NONE, 0},
      {(int)TreeTypeIndex::BOREAL, 6},
      {(int)TreeTypeIndex::TEMPERATENEEDLE, 6},
      {(int)TreeTypeIndex::TEMPERATEMIXED, 5},
      {(int)TreeTypeIndex::SPARSE, 3},
      {(int)TreeTypeIndex::TROPICALDRY, 28},
      {(int)TreeTypeIndex::TROPICALMOIST, 29}}},
    {"riversEu4",
     {{(int)0, 0},
      {(int)1, 6},
      {(int)2, 6},
      {(int)TreeTypeIndex::TEMPERATEMIXED, 5},
      {(int)TreeTypeIndex::SPARSE, 3},
      {(int)TreeTypeIndex::TROPICALDRY, 28},
      {(int)TreeTypeIndex::TROPICALMOIST, 29}}}

};

ImageExporter::ImageExporter() {}

ImageExporter::ImageExporter(const std::string &gamePath,
                             const std::string &gameTag)
    : gamePath{gamePath}, gameTag{gameTag} {}

ImageExporter::~ImageExporter() {}

void ImageExporter::writeBufferPixels(std::vector<unsigned char> &pixels,
                                      int index, const Fwg::Gfx::Colour &colour,
                                      unsigned char alphaValue) {
  pixels[index] = colour.getRed();
  pixels[index + 1] = colour.getGreen();
  pixels[index + 2] = colour.getBlue();
  pixels[index + 3] = alphaValue;
}

Bitmap ImageExporter::cutBaseMap(const std::string &path, const double factor,
                                 const int bit) const {
  auto &conf = Cfg::Values();
  std::string sourceMap{conf.loadMapsPath + path};
  Fwg::Utils::Logging::logLine("CUTTING mode: Cutting Map from ", sourceMap);
  Bitmap baseMap = Fwg::IO::Reader::readGenericImage(sourceMap, conf);
  auto cutBase = Bmp::cut(baseMap, conf.minX * factor, conf.maxX * factor,
                          conf.minY * factor, conf.maxY * factor, factor);
  if (conf.scale) {
    cutBase = Bmp::scale(cutBase, conf.scaleX * factor, conf.scaleY * factor,
                         conf.keepRatio);
  }
  return cutBase;
}

void ImageExporter::dump8BitHeightmap(const std::vector<float> &altitudeData,
                                      const std::string &path,
                                      const std::string &colourMapKey) const {
  Utils::Logging::logLine("ImageExporter::Copying heightmap to ",
                          Fwg::Utils::userFilter(path, Cfg::Values().username));
  auto &conf = Cfg::Values();
  auto heightMap = Fwg::Gfx::Bitmap(conf.width, conf.height, 24, altitudeData);
  if (!heightMap.initialised()) {
    Utils::Logging::logLine("ImageExporter::Heightmap not yet initialised");
    return;
  }
  if (heightMap.size() != conf.bitmapSize) {
    Utils::Logging::logLine("ImageExporter::Heightmap size mismatch");
    return;
  }
  if (gameTag == "Vic3") {
    // need to scale to default vic3 map sizes, due to their compression
    int width = heightMap.width();
    int height = heightMap.height();
    //  save the bmp as a png
    auto h2 = Bmp::scale(heightMap, width * 2, height * 2, false);
    Png::save(h2, path + ".png", true, LCT_GREY, 16);
    heightMap = h2;
  } else {
    Bitmap outputMap(Cfg::Values().width, Cfg::Values().height, 8);
    outputMap.colourtable = colourTables.at(colourMapKey + gameTag);
    // now map from 24 bit climate map
    for (int i = 0; i < Cfg::Values().bitmapSize; i++)
      outputMap.setColourAtIndex(i, outputMap.lookUp(heightMap[i].getRed()));

    Bmp::save8bit(outputMap, path + ".bmp");
  }
}

void ImageExporter::dump8BitTerrain(
    const Fwg::Terrain::TerrainData &terrainData,
    const Fwg::Climate::ClimateData &climateIn,
    const Arda::Civilization::CivilizationLayer &civLayer,
    const std::string &path, const std::string &colourMapKey,
    const bool cut) const {
  Utils::Logging::logLine("ImageExporter::Writing terrain to ",
                          Fwg::Utils::userFilter(path, Cfg::Values().username));
  auto &conf = Cfg::Values();
  if (climateIn.climates.size() != conf.bitmapSize ||
      climateIn.treeCoverage.size() != conf.bitmapSize ||
      climateIn.dominantForest.size() != conf.bitmapSize ||
      terrainData.landForms.size() != conf.bitmapSize) {
    Utils::Logging::logLine("ImageExporter::Climate data size mismatch");
    // print all the sizes
    Utils::Logging::logLine("Climates size: ", climateIn.climates.size());
    Utils::Logging::logLine("TreeCoverage size: ",
                            climateIn.treeCoverage.size());
    Utils::Logging::logLine("Dominant Forest size: ",
                            climateIn.dominantForest.size());
    Utils::Logging::logLine("Landforms size: ", terrainData.landForms.size());

    return;
  }

  Bitmap hoi4terrain(conf.width, conf.height, 8);
  hoi4terrain.colourtable = colourTables.at(colourMapKey + gameTag);
  // hoi4terrain.colourtable = colourTables.at(colourMapKey + gameTag);
  if (cut) {
    hoi4terrain = cutBaseMap("//terrain.bmp");
  } else {

    for (auto i = 0; i < conf.bitmapSize; i++) {
      int elevationMod = 0;
      const auto &elevationType = terrainData.landForms.at(i).landForm;
      if (elevationType == Terrain::ElevationTypeIndex::HILLS) {
        elevationMod = 100;
      } else if (elevationType == Terrain::ElevationTypeIndex::MOUNTAINS) {
        elevationMod = 200;
      } else if (elevationType == Terrain::ElevationTypeIndex::PEAKS) {
        elevationMod = 300;
      } else if (elevationType == Terrain::ElevationTypeIndex::STEEPPEAKS) {
        elevationMod = 300;
      } else if (elevationType == Terrain::ElevationTypeIndex::CLIFF) {
        elevationMod = 200;
      } else if (elevationType == Terrain::ElevationTypeIndex::LOWHILLS) {
        elevationMod = 100;
      }

      auto primaryClimateType = (int)climateIn.climates[i].getChances(0).second;
      hoi4terrain.setColourAtIndex(
          i, hoi4terrain.lookUp(indexMaps.at(colourMapKey + gameTag)
                                    .at(elevationMod + primaryClimateType)));
      auto treeType = (int)climateIn.treeCoverage[i];
      if (treeType && climateIn.dominantForest[i]) {
        hoi4terrain.setColourAtIndex(
            i, hoi4terrain.lookUp(
                   indexMaps.at("tree" + colourMapKey + gameTag).at(treeType)));
      }
    }
    auto cityIndices =
        civLayer.getAll(Arda::Civilization::TopographyType::CITY);
    for (auto cityIndex : cityIndices) {
      hoi4terrain.setColourAtIndex(cityIndex, hoi4terrain.lookUp(13));
    }
    auto farmIndices =
        civLayer.getAll(Arda::Civilization::TopographyType::FARMLAND);
    for (auto farmIndex : farmIndices) {
      hoi4terrain.setColourAtIndex(farmIndex, hoi4terrain.lookUp(5));
    }
  }
  Bmp::save8bit(hoi4terrain, path);
}

void ImageExporter::dump8BitRivers(const Fwg::Terrain::TerrainData &terrainData,
                                   const Fwg::Climate::ClimateData &climateIn,
                                   const std::string &path,
                                   const std::string &colourMapKey,
                                   const bool cut) const {
  Utils::Logging::logLine("ImageExporter::Writing rivers to ",
                          Fwg::Utils::userFilter(path, Cfg::Values().username));

  Bitmap riverMap(Cfg::Values().width, Cfg::Values().height, 8);
  riverMap.colourtable = colourTables.at(colourMapKey + gameTag);
  if (!cut) {
    for (auto i = 0; i < riverMap.size(); i++) {
      if (terrainData.landForms.at(i).altitude > 0.0) {
        riverMap.setColourAtIndex(i, riverMap.lookUp(255));
      } else {
        riverMap.setColourAtIndex(i, riverMap.lookUp(254));
      }
    }
    for (auto &river : climateIn.rivers) {
      for (int i = 0; i < river.pixels.size(); i++) {
        riverMap.setColourAtIndex(
            river.pixels[i],
            riverMap.lookUp(3 + static_cast<int>(river.getWeight(i) * 8.0)));
      }

      if (!river.isTributaryRiver()) {
        riverMap.setColourAtIndex(river.getSource(), riverMap.lookUp(0));
      } else {
        riverMap.setColourAtIndex(river.getCurrentEnd(), riverMap.lookUp(1));
      }
    }
  } else {
    riverMap = cutBaseMap("//rivers.bmp");
  }
  if (gameTag == "Vic3") {
    auto scaledMap = Bmp::scale(riverMap, 8192, 3616, false);
    Png::save(scaledMap, path + ".png");
  } else {
    Bmp::save8bit(riverMap, path + ".bmp");
  }
}

void ImageExporter::dump8BitTrees(const Fwg::Terrain::TerrainData &terrainData,
                                  const Fwg::Climate::ClimateData &climateIn,
                                  const std::string &path,
                                  const std::string &colourMapKey,
                                  const bool cut) const {
  auto &conf = Cfg::Values();
  Utils::Logging::logLine("ImageExporter::Writing trees to ",
                          Fwg::Utils::userFilter(path, conf.username));
  const double width = conf.width;
  constexpr auto factor = 3.4133333333333333333333333333333;
  Bitmap trees(((double)conf.width / factor), ((double)conf.height / factor),
               8);
  trees.colourtable = colourTables.at(colourMapKey + gameTag);
  // we have to remove all coastal trees, as the downscaling can cause trees to
  // appear in the water
  auto treeCoverage = climateIn.treeCoverage;

  // check if treeCoverage is valid
  if (treeCoverage.size() != conf.bitmapSize) {
    Utils::Logging::logLine("ImageExporter::Tree coverage size mismatch");
    return;
  }
  // check if dominantForest is valid
  if (climateIn.dominantForest.size() != conf.bitmapSize) {
    Utils::Logging::logLine("ImageExporter::Dominant forest size mismatch");
    return;
  }

  std::vector<int> offsets = {1, -1, conf.width,     -conf.width,
                              2, -2, 2 * conf.width, -2 * conf.width};
  for (auto i = 0; i < treeCoverage.size(); i++) {
    for (auto offset : offsets) {
      if (i + offset < treeCoverage.size() && i + offset >= 0) {
        if (terrainData.landForms.at(i + offset).altitude <= 0.0) {
          treeCoverage[i] = Fwg::Climate::Detail::TreeTypeIndex::NONE;
        }
      }
    }
  }

  if (!cut) {
    for (auto i = 0; i < trees.height(); i++) {
      for (auto w = 0; w < trees.width(); w++) {
        double refHeight = ceil((double)i * factor);
        double refWidth =
            std::clamp((double)w * factor, 0.0, (double)Cfg::Values().width);
        auto treeType = (int)treeCoverage[refHeight * width + refWidth];
        if (climateIn.dominantForest[refHeight * width + refWidth]) {
          // map the colour from
          trees.setColourAtIndex(
              i * trees.width() + w,
              trees.lookUp(indexMaps.at(colourMapKey + gameTag).at(treeType)));
        }
      }
    }
  } else {
    trees = cutBaseMap("//trees.bmp", (1.0 / factor));
  }
  Bmp::save8bit(trees, path);
}

void ImageExporter::dumpDDSFiles(const std::vector<float> &heightMap,
                                 const std::string &path, const bool cut,
                                 const int maxFactor) const {
  Utils::Logging::logLine("ImageExporter::Writing DDS files");
  using namespace DirectX;
  const auto &width = Cfg::Values().width;

  for (auto factor = 2, counter = 0; factor <= maxFactor;
       factor *= 2, counter++) {
    auto tempPath{path};
    if (gameTag == "Hoi4")
      tempPath += std::to_string(counter);
    tempPath += ".dds";
    auto imageWidth = width / factor;
    auto imageHeight = Cfg::Values().height / factor;
    std::vector<uint8_t> pixels(imageWidth * imageHeight * 4, 0);

    for (auto h = 0; h < imageHeight; h++) {
      for (auto w = 0; w < imageWidth; w++) {
        auto referenceIndex = factor * h * width + factor * w;
        auto depth = heightMap[referenceIndex] / (float)Cfg::Values().seaLevel;
        auto imageIndex =
            imageHeight * imageWidth - (h * imageWidth + (imageWidth - w));
        imageIndex *= 4;
        if (heightMap[referenceIndex] <= Cfg::Values().seaLevel) {
          pixels[imageIndex] = static_cast<unsigned char>(200.0 * depth);
          pixels[imageIndex + 1] = static_cast<unsigned char>(150.0 * depth);
          pixels[imageIndex + 2] = static_cast<unsigned char>(100.0 * depth);
          pixels[imageIndex + 3] = 255;
        } else {
          pixels[imageIndex] = 100;
          pixels[imageIndex + 1] = 100;
          pixels[imageIndex + 2] = 50;
          pixels[imageIndex + 3] = 255;
        }
      }
    }
    Utils::Logging::logLine(
        "ImageExporter::Writing DDS files to ",
        Fwg::Utils::userFilter(tempPath, Cfg::Values().username));
    Arda::Gfx::Textures::writeDDS(imageWidth, imageHeight, pixels,
                                  DXGI_FORMAT_B8G8R8A8_UNORM, tempPath);
  }
}

void ImageExporter::dumpTerrainColourmap(
    const Bitmap &climateMap,
    const Arda::Civilization::CivilizationLayer &civLayer,
    const std::string &modPath, const std::string &mapName,
    const DXGI_FORMAT format, int scaleFactor, const bool cut) const {
  auto &cfg = Cfg::Values();
  Utils::Logging::logLine("ImageExporter::Writing terrain colourmap to ",
                          Utils::userFilter(modPath + mapName, cfg.username));

  // check if all inputs are valid
  if (climateMap.size() != cfg.bitmapSize) {
    Utils::Logging::logLine("ImageExporter::Climate map size mismatch");
    return;
  }
  // if (civLayer.urbanisation.size() != cfg.bitmapSize) {
  //   Utils::Logging::logLine("ImageExporter::Urbanisation size mismatch");
  //   return;
  // }

  const auto &height = climateMap.height();
  const auto &width = climateMap.width();
  int factor = scaleFactor;
  auto imageWidth = width / factor;
  auto imageHeight = height / factor;

  std::vector<uint8_t> pixels(imageWidth * imageHeight * 4, 0);
  auto cityIndices = civLayer.getAll(Arda::Civilization::TopographyType::CITY);

  if (!cut) {
    for (auto h = 0; h < imageHeight; h++) {
      for (auto w = 0; w < imageWidth; w++) {
        auto colourmapIndex = factor * h * width + factor * w;
        const auto &c = climateMap[colourmapIndex];
        auto imageIndex =
            imageHeight * imageWidth - (h * imageWidth + (imageWidth - w));
        imageIndex *= 4;
        pixels[imageIndex] = c.getBlue();
        pixels[imageIndex + 1] = c.getGreen();
        pixels[imageIndex + 2] = c.getRed();
        if (gameTag == "Eu4" || gameTag == "Vic3") {
          pixels[imageIndex + 3] = 255;
        }
      }
    }
    if (gameTag == "Hoi4") // add city lights
    {
      for (int cityIndex : cityIndices) {
        int cityY = cityIndex / width;
        int cityX = cityIndex % width;

        // Adjust for factor (if climate map < image size)
        int imageY = cityY / factor;
        int imageX = cityX / factor;

        // Flip Y if image is stored bottom-up
        int flippedY = imageHeight - imageY - 1;

        int imageIndex = (flippedY * imageWidth + imageX) * 4;

        // Bright city lights: set alpha based on some heuristic
        pixels[imageIndex + 3] = 255; // or e.g. 128 for dimmer lights
      }
    }

  } else {
    // load base game colourmap
    pixels = Arda::Gfx::Textures::readDDS(gamePath + mapName);
    int maxY = static_cast<int>(1024 - cfg.maxY / (double)factor);
    int minY = static_cast<int>(1024 - cfg.minY / (double)factor);
    int maxX = static_cast<int>(cfg.maxX / (double)factor);
    int minX = static_cast<int>(cfg.minX / (double)factor);
    std::swap(minY, maxY);
    // cut it and reassign it
    pixels = Utils::cutBuffer(pixels, 2816, 1024, minX, maxX, minY, maxY, 4);
    if (cfg.scale) {
      pixels = Utils::scaleBuffer(pixels, abs(maxX - minX), abs(maxY - minY),
                                  cfg.scaleX / factor, cfg.scaleY / factor, 4,
                                  cfg.keepRatio);
    }
  }
  if (gameTag == "Vic3") {
    Arda::Gfx::Textures::writeMipMapDDS(imageWidth, imageHeight, pixels, format,
                                        modPath + mapName, true);
  } else {

    if (cfg.scale)
      Arda::Gfx::Textures::writeDDS(cfg.scaleX / factor, cfg.scaleY / factor,
                                    pixels, format, modPath + mapName);
    else
      Arda::Gfx::Textures::writeDDS(imageWidth, imageHeight, pixels, format,
                                    modPath + mapName);
  }
}

void ImageExporter::dumpWorldNormal(const Bitmap &sobelMap,
                                    const std::string &path,
                                    const bool cut) const {
  Utils::Logging::logLine("ImageExporter::Writing normalMap to ",
                          Fwg::Utils::userFilter(path, Cfg::Values().username));
  int factor = 2; // image width and height are halved

  Bmp::save(Fwg::Gfx::Bmp::scaleInterpolation(sobelMap, factor),
            (path).c_str());
}

} // namespace Rpx::Gfx