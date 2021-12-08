#include "FormatConverter.h"

void FormatConverter::dump8BitHeightmap(string path, string colourMapKey)
{
	Bitmap hoi4Heightmap(Data::getInstance().width, Data::getInstance().height, 8);
	hoi4Heightmap.getColourtable() = colourTables[colourMapKey];

	// now map from 24 bit climate map
	auto heightmap = Data::getInstance().findBitmapByKey("heightmap");
	for (int i = 0; i < Data::getInstance().bitmapSize; i++)
	{
		hoi4Heightmap.bit8Buffer[i] = heightmap.getColourAtIndex(i).getRed();
	}
	Bitmap::SaveBMPToFile(hoi4Heightmap, (path).c_str());
}

void FormatConverter::dump8BitTerrain(string path, string colourMapKey)
{
	Bitmap hoi4terrain(Data::getInstance().width, Data::getInstance().height, 8);
	hoi4terrain.getColourtable() = colourTables[colourMapKey];

	// now map from 24 bit climate map
	auto climate = Data::getInstance().findBitmapByKey("climate");
	for (int i = 0; i < Data::getInstance().bitmapSize; i++)
	{
		hoi4terrain.bit8Buffer[i] = colourMaps[colourMapKey][climate.getColourAtIndex(i)];
	}
	Bitmap::SaveBMPToFile(hoi4terrain, (path).c_str());
}

void FormatConverter::dump8BitCities(string path, string colourMapKey)
{
	Bitmap cities(Data::getInstance().width, Data::getInstance().height, 8);
	cities.getColourtable() = colourTables[colourMapKey];

	auto climate = Data::getInstance().findBitmapByKey("climate");
	for (int i = 0; i < Data::getInstance().bitmapSize; i++)
	{
		if (climate.getColourAtIndex(i) == Data::getInstance().namedColours["sea"])
		{
			cities.bit8Buffer[i] = 32;
		}
		else
			cities.bit8Buffer[i] = 18;
	}
	Bitmap::SaveBMPToFile(cities, (path).c_str());
}

void FormatConverter::dump8BitRivers(string path, string colourMapKey)
{
	Bitmap rivers(Data::getInstance().width, Data::getInstance().height, 8);
	rivers.getColourtable() = colourTables[colourMapKey];

	auto rivers2 = Data::getInstance().findBitmapByKey("rivers");
	for (int i = 0; i < Data::getInstance().bitmapSize; i++)
	{
		rivers.bit8Buffer[i] = colourMaps[colourMapKey][rivers2.getColourAtIndex(i)];
	}
	Bitmap::SaveBMPToFile(rivers, (path).c_str());
}

void FormatConverter::dump8BitTrees(string path, string colourMapKey)
{
	Bitmap trees(Data::getInstance().width, Data::getInstance().height, 8);
	trees.getColourtable() = colourTables[colourMapKey];

	auto climate = Data::getInstance().findBitmapByKey("climate");
	for (int i = 0; i < trees.bInfoHeader.biSizeImage; i++)
	{
		trees.bit8Buffer[i] = colourMaps[colourMapKey][climate.getColourAtIndex(i)];
	}
	Bitmap::SaveBMPToFile(trees, (path).c_str());
}

void FormatConverter::dumpWorldNormal(string path)
{
	auto height = Data::getInstance().height;
	auto width = Data::getInstance().width;
	Bitmap normalMap(width, height, 24);
	auto heightBMP = Data::getInstance().findBitmapByKey("heightmap");

	//for (int i = 0; i < Data::getInstance().bitmapSize; i++)
	//{
	//	normalMap.setColourAtIndex(i, Colour(1,100,1));
	//}
	normalMap.setBuffer(heightBMP.sobelFilter());
	Bitmap::SaveBMPToFile(normalMap, (path).c_str());

}

void FormatConverter::dumpDDSFiles(string path)
{
	using namespace DirectX;

	auto riverBMP = Data::getInstance().findBitmapByKey("rivers");
	auto heightBMP = Data::getInstance().findBitmapByKey("heightmap");

	for (int factor = 1, counter = 0; factor <= 4; factor *= 2, counter++)
	{
		auto tempPath = path;
		tempPath += to_string(counter);
		tempPath += ".dds";
		wstring destination = wstring(tempPath.begin(), tempPath.end());
		Image image;
		image.width = Data::getInstance().width / factor;
		image.height = Data::getInstance().height / factor;
		image.format = DXGI_FORMAT_B8G8R8A8_UNORM;
		image.rowPitch = sizeof(uint8_t) * image.width * 4;
		image.slicePitch = sizeof(uint8_t) * image.width * image.height * 4;
		vector<uint8_t> pixels(image.width*image.height * 4, 0);
		auto vectorSize = image.width * image.height * 4;
		cout << vectorSize << endl;
		for (int i = 0; i < image.width * image.height; i++)
		{
			uint32_t width = i % image.width;
			uint32_t height = i / image.width;
			auto refIndex = height*image.width*factor*factor + width*factor;
			double depth = (double)heightBMP.getColourAtIndex(refIndex).getBlue() / (double)Data::getInstance().seaLevel;
			auto index = ((image.height - 1 - height)*image.width + width) * 4;

			if (riverBMP.getColourAtIndex(refIndex) == Data::getInstance().namedColours["sea"])
			{
				pixels[index] = 49 * depth;
				pixels[index + 1] = 24 * depth;
				pixels[index + 2] = 16 * depth;
				pixels[index + 3] = 255;
			}
			else
			{
				pixels[index] = 100;
				pixels[index + 1] = 100;
				pixels[index + 2] = 50;
				pixels[index + 3] = 255;
			}
		}
		image.pixels = pixels.data();
		SaveToDDSFile(image, DDS_FLAGS_NONE, destination.c_str());
	}
}

FormatConverter::FormatConverter()
{
	string terrainsourceString = ("resources\\hoi4\\terrain.bmp");
	Bitmap terrain = Bitmap::Load8bitBMP(terrainsourceString.c_str(), "terrain");
	colourTables["terrainHoi4"] = terrain.getColourtable();

	string citySource = ("resources\\hoi4\\terrain.bmp");
	Bitmap cities = Bitmap::Load8bitBMP(citySource.c_str(), "cities");
	colourTables["citiesHoi4"] = cities.getColourtable();

	string riverSource = ("resources\\hoi4\\rivers.bmp");
	Bitmap rivers = Bitmap::Load8bitBMP(riverSource.c_str(), "rivers");
	colourTables["riversHoi4"] = rivers.getColourtable();

	string treeSource = ("resources\\hoi4\\trees.bmp");
	Bitmap trees = Bitmap::Load8bitBMP(treeSource.c_str(), "trees");
	colourTables["treesHoi4"] = trees.getColourtable();

	string heightmapSource = ("resources\\hoi4\\heightmap.bmp");
	Bitmap heightmap = Bitmap::Load8bitBMP(heightmapSource.c_str(), "heightmap");
	colourTables["heightmapHoi4"] = heightmap.getColourtable();
}


FormatConverter::~FormatConverter()
{
}




