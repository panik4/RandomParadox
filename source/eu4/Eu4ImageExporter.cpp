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
  std::string mapFolderName = "//map";
  std::string terrainsourceString =
      (gamePath + mapFolderName + "//terrain.bmp");
  Image terrain =
      Fwg::IO::Reader::readGenericImage(terrainsourceString, conf, false);
  colourTables["terrain" + gameTag] = terrain.colourtable;

  std::string citySource = (gamePath + mapFolderName + "//terrain.bmp");
  Image cities = Fwg::IO::Reader::readGenericImage(citySource, conf, false);
  colourTables["cities" + gameTag] = cities.colourtable;

  std::string riverSource = (gamePath + mapFolderName + "//rivers.bmp");
  Image rivers = Fwg::IO::Reader::readGenericImage(riverSource, conf, false);
  colourTables["rivers" + gameTag] = rivers.colourtable;

  std::string treeSource = (gamePath + mapFolderName + "//trees.bmp");
  auto cut = conf.cut;
  conf.cut = false;
  Image trees = Fwg::IO::Reader::readGenericImage(treeSource, conf, false);
  conf.cut = cut;
  colourTables["trees" + gameTag] = trees.colourtable;

  std::string heightmapSource = (gamePath + mapFolderName + "//heightmap.bmp");
  Image heightmap =
      Fwg::IO::Reader::readGenericImage(heightmapSource, conf, false);
  colourTables["heightmap" + gameTag] = heightmap.colourtable;
}

ImageExporter::~ImageExporter() {}
} // namespace Rpx::Gfx::Eu4