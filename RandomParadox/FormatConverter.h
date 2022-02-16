#pragma once
#include <map>
#include "FastWorldGen/FastWorldGen/utils/Data.h"
#include "FastWorldGen/FastWorldGen/entities/Colour.h"
#include "generic/TextureWriter.h"
#include "DirectXTex.h"

class FormatConverter
{
	// map of maps of colours, defines which FastWorldGen colour 
	// should be mapped to which game compatible colour
	std::map<std::string, std::map<Colour, int>> colourMaps{
		{"terrainHoi4", {
			{ Data::getInstance().namedColours["grassland"], 0 },
			{ Data::getInstance().namedColours["ice"], 19 },
			{ Data::getInstance().namedColours["tundra"], 9 },
			{ Data::getInstance().namedColours["forest"], 1 },
			{ Data::getInstance().namedColours["jungle"], 21 },
			{ Data::getInstance().namedColours["savannah"], 0 },
			{ Data::getInstance().namedColours["desert"], 7 },
			{ Data::getInstance().namedColours["peaks"], 16 },
			{ Data::getInstance().namedColours["mountains"], 11 },
			{ Data::getInstance().namedColours["lowMountains"], 20 },
			{ Data::getInstance().namedColours["sea"], 15 }
		}
	},
		{"riversHoi4",{
			{ Data::getInstance().namedColours["land"], 255 },
			{ Data::getInstance().namedColours["river"], 3 },
			{ Data::getInstance().namedColours["river"] * 0.9, 3 },
			{ Data::getInstance().namedColours["river"] * 0.8, 6 },
			{ Data::getInstance().namedColours["river"] * 0.7, 6 },
			{ Data::getInstance().namedColours["river"] * 0.6, 10 },
			{ Data::getInstance().namedColours["river"] * 0.5, 11 },
			{ Data::getInstance().namedColours["river"] * 0.4, 11 },
			{ Data::getInstance().namedColours["sea"], 254 },
			{ Data::getInstance().namedColours["riverStart"], 0 },
			{ Data::getInstance().namedColours["riverStartTributary"], 3 },
			{ Data::getInstance().namedColours["riverEnd"], 1 }
		}
	},
	{ "treesHoi4",{
			{ Data::getInstance().namedColours["grassland"], 0 },
			{ Data::getInstance().namedColours["ice"], 0 },
			{ Data::getInstance().namedColours["tundra"], 0 },
			{ Data::getInstance().namedColours["forest"], 6 },
			{ Data::getInstance().namedColours["jungle"], 28 },
			{ Data::getInstance().namedColours["savannah"], 0 },
			{ Data::getInstance().namedColours["desert"], 0 },
			{ Data::getInstance().namedColours["peaks"], 0 },
			{ Data::getInstance().namedColours["mountains"], 0 },
			{ Data::getInstance().namedColours["lowMountains"], 0 },
			{ Data::getInstance().namedColours["sea"], 0 }
	}
	}
	};
	std::map<std::string, std::vector<unsigned char>> colourTables;

public:
	// constructor/destructor
	FormatConverter(std::string hoiPath);
	~FormatConverter();
	// member functions
	void dump8BitHeightmap(std::string path, std::string colourMapKey) const;
	void dump8BitTerrain(std::string path, std::string colourMapKey) const;
	void dump8BitCities(std::string path, std::string colourMapKey) const;
	void dump8BitRivers(std::string path, std::string colourMapKey) const;
	void dump8BitTrees(std::string path, std::string colourMapKey) const; // width/4, height/4
	void dumpDDSFiles(std::string path) const;
	void dumpTerrainColourmap(std::string path) const;
	void dumpWorldNormal(std::string path) const; // width/2, height/2
};

