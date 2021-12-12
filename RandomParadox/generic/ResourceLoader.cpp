#include "ResourceLoader.h"

ResourceLoader::ResourceLoader()
{
}

ResourceLoader::~ResourceLoader()
{
}

Bitmap ResourceLoader::loadProvinceMap(std::string gamePath)
{
	return Bitmap::Load24bitBMP((gamePath + ("\\map\\provinces.bmp")).c_str(), "provinces");
}

Bitmap ResourceLoader::loadHeightMap(std::string gamePath)
{
	return Bitmap::Load8bitBMP((gamePath + ("\\map\\heightmap.bmp")).c_str(), "heightmap");
}

vector<std::string> ResourceLoader::loadStates(std::string gamePath)
{
	return ParserUtils::readFilesInDirectory(gamePath + "\\history\\states\\");
}

vector<std::string> ResourceLoader::loadDefinition(std::string gamePath)
{
	return ParserUtils::getLines(gamePath + "\\map\\definition.csv");
}
