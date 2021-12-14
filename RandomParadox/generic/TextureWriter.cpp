#include "TextureWriter.h"

TextureWriter::TextureWriter()
{
}

TextureWriter::~TextureWriter()
{
}

void TextureWriter::writeDDS(int width, int height, vector<uint8_t> pixelData, DXGI_FORMAT format, std::string destination)
{
	wstring destinationPath = wstring(destination.begin(), destination.end());
	Image image;
	image.width = width;
	image.height = height;
	image.format = format;
	image.rowPitch = sizeof(uint8_t) * image.width * 4;
	image.slicePitch = sizeof(uint8_t) * image.width * image.height;
	image.pixels = pixelData.data();
	SaveToDDSFile(image, DDS_FLAGS_NONE, destinationPath.c_str());
}

void TextureWriter::writeTGA(int width, int height, vector<uint8_t> pixelData, std::string destination)
{
	wstring destinationPath = wstring(destination.begin(), destination.end());
	Image image;
	image.width = width;
	image.height = height;
	image.format = DXGI_FORMAT_B8G8R8A8_UNORM;
	image.rowPitch = sizeof(uint8_t) * image.width * 4;
	image.slicePitch = sizeof(uint8_t) * image.width * image.height;
	image.pixels = pixelData.data();
	SaveToTGAFile(image, TGA_FLAGS_NONE, destinationPath.c_str());
}

