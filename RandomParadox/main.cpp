#include "FastWorldGen/FastWorldGen/entities/Colour.h"
#include "FastWorldGen/FastWorldGen/utils/Bitmap.h"
#include "FastWorldGen/FastWorldGen/FastWorldGenerator.h"
#include "FormatConverter.h"
#include "Hoi4Parser.h"
using namespace std;
int main() {
	string modPath = "pdoxMod\\";
	FastWorldGenerator f;
	f.generateWorld();

	//Bitmap::SaveBMPToFile(Data::getInstance().findBitmapByKey("heightmap"), (modPath + ("\\map\\heightmap.bmp")).c_str());
	Bitmap::SaveBMPToFile(Data::getInstance().findBitmapByKey("provinces"), (modPath + ("\\map\\provinces.bmp")).c_str());



	// HOI 4:
	FormatConverter formatConverter;
	//formatConverter.dump8BitTerrain(modPath + "\\map\\terrain.bmp", "terrainHoi4");
	//formatConverter.dump8BitCities(modPath + "\\map\\cities.bmp", "citiesHoi4");
	//formatConverter.dump8BitRivers(modPath + "\\map\\rivers.bmp", "riversHoi4");
	formatConverter.dump8BitTrees(modPath + "\\map\\trees.bmp", "treesHoi4");
	formatConverter.dump8BitHeightmap(modPath + "\\map\\heightmap.bmp", "heightmapHoi4");
	formatConverter.dumpDDSFiles(modPath + "\\map\\terrain\\colormap_water_");
	formatConverter.dumpWorldNormal(modPath + "\\map\\world_normal.bmp");


	Hoi4Parser hoiParse;
	hoiParse.dumpAdj(modPath+ "\\map\\adjacencies.csv");
	hoiParse.dumpAirports(modPath + "\\map\\airports.txt", f.provinceGenerator.regions);
	hoiParse.dumpBuildings(modPath + "\\map\\buildings.txt", f.provinceGenerator.regions);
	hoiParse.dumpContinents(modPath + "\\map\\continents.txt", f.provinceGenerator.continents);
	hoiParse.dumpDefinition(modPath + "\\map\\definition.csv", f.provinceGenerator.provinces);
	hoiParse.dumpRocketSites(modPath + "\\map\\rocketsites.txt", f.provinceGenerator.regions);
	
	return 0;
}