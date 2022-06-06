#include "generic/GenericModule.h"

void GenericModule::createPaths(std::string basePath) { // mod directory
  std::filesystem::create_directory(basePath);
  // map
  std::filesystem::remove_all(basePath + "\\map\\");
  std::filesystem::remove_all(basePath + "\\gfx");
  std::filesystem::remove_all(basePath + "\\history");
  std::filesystem::remove_all(basePath + "\\common\\");
  std::filesystem::remove_all(basePath + "\\localisation\\");
  std::filesystem::create_directory(basePath + "\\map\\");
  std::filesystem::create_directory(basePath + "\\map\\terrain\\");
  // gfx
  std::filesystem::create_directory(basePath + "\\gfx\\");
  std::filesystem::create_directory(basePath + "\\gfx\\flags\\");
  // history
  std::filesystem::create_directory(basePath + "\\history\\");
  std::filesystem::create_directory(basePath + "\\history\\countries\\");
  // localisation
  std::filesystem::create_directory(basePath + "\\localisation\\");
  // common
  std::filesystem::create_directory(basePath + "\\common\\");
  std::filesystem::create_directory(basePath + "\\common\\countries\\");
  std::filesystem::create_directory(basePath + "\\common\\bookmarks\\");
  std::filesystem::create_directory(basePath + "\\common\\country_tags\\");
}
