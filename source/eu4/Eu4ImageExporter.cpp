#include "eu4/Eu4ImageExporter.h"
namespace Rpx::Gfx::Eu4 {
using namespace Arda::Gfx::Textures;
using namespace Fwg;
using namespace Fwg::Gfx;

ImageExporter::ImageExporter() {}

ImageExporter::ImageExporter(const std::string &gamePath,
                                 const std::string &gameTag)
    : Rpx::Gfx::ImageExporter(gamePath, gameTag) {
  auto &conf = Cfg::Values();
  std::string mapFolderName = "/map/";
  std::string terrainsourceString =
      (gamePath + mapFolderName + "/terrain.bmp");
  Image terrain =
      Fwg::IO::Reader::readGenericImage(terrainsourceString, conf.imageReadContext(), false);
  colourTables["terrain" + gameTag] = terrain.colourtable;

  std::string citySource = (gamePath + mapFolderName + "cities.bmp");
  Image cities = Fwg::IO::Reader::readGenericImage(citySource, conf.imageReadContext(), false);
  colourTables["cities" + gameTag] = cities.colourtable;

  std::string riverSource = (gamePath + mapFolderName + "rivers.bmp");
  Image rivers = Fwg::IO::Reader::readGenericImage(riverSource, conf.imageReadContext(), false);
  colourTables["rivers" + gameTag] = rivers.colourtable;

  std::string treeSource = (gamePath + mapFolderName + "trees.bmp");
  auto cut = conf.imageImportConfig.cut;
  conf.imageImportConfig.cut = false;
  Image trees = Fwg::IO::Reader::readGenericImage(treeSource, conf.imageReadContext(), false);
  conf.imageImportConfig.cut = cut;
  colourTables["trees" + gameTag] = trees.colourtable;

  std::string heightmapSource = (gamePath + mapFolderName + "heightmap.bmp");
  Image heightmap =
      Fwg::IO::Reader::readGenericImage(heightmapSource, conf.imageReadContext(), false);
  colourTables["heightmap" + gameTag] = heightmap.colourtable;
}

ImageExporter::~ImageExporter() {}
} // namespace Rpx::Gfx::Eu4
