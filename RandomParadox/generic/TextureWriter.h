#pragma once
#include "../FastWorldGen/FastWorldGen/utils/Data.h"
#include "../FastWorldGen/FastWorldGen/entities/Colour.h"
#include "DirectXTex.h"

using namespace DirectX;
class TextureWriter
{
public:
	TextureWriter();
	~TextureWriter();

	static void writeDDS(int width, int height, vector<uint8_t> pixelData, DXGI_FORMAT format, std::string destination);
	static void writeTGA(int width, int height, vector<uint8_t> pixelData, std::string destination);
};

