#include "ResourceLoader.h"




ResourceLoader::ResourceLoader()
{
}

ResourceLoader::ResourceLoader(bool genHoi4, std::string hoi4Path) : genHoi4{genHoi4}, hoi4Path{hoi4Path}
{
}

ResourceLoader::~ResourceLoader()
{
}

Bitmap ResourceLoader::loadProvinceMap()
{
	return Bitmap::Load24bitBMP((hoi4Path + ("\\map\\provinces.bmp")).c_str(), "provinces");
}

Bitmap ResourceLoader::loadHeightMap()
{
	return Bitmap::Load8bitBMP((hoi4Path + ("\\map\\heightmap.bmp")).c_str(), "heightmap");
}

vector<std::string> ResourceLoader::loadStates()
{
	return ParserUtils::readFilesInDirectory(hoi4Path + "\\history\\states\\");
}


vector<std::string> ResourceLoader::loadDefinition()
{
	return ParserUtils::getLines(hoi4Path + "\\map\\definition.csv");
}
