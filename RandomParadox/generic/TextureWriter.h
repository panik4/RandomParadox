#pragma once
#include "DirectXTex.h"
#include "../FastWorldGen/FastWorldGen/utils/Data.h"
#include "../FastWorldGen/FastWorldGen/entities/Colour.h"

using namespace DirectX;
class TextureWriter
{
public:
	static void writeDDS(int width, int height, std::vector<uint8_t>& pixelData, DXGI_FORMAT format, std::string destination);
	static void writeTGA(int width, int height, std::vector<uint8_t>& pixelData, std::string destination);
	static std::vector<uint8_t> readTGA(std::string destination);
};

