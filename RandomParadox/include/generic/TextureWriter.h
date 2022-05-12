#pragma once
#include "FastWorldGenerator.h"
#include "DirectXTex.h"

using namespace DirectX;
class TextureWriter {
public:
  static void writeDDS(int width, int height, std::vector<uint8_t> &pixelData,
                       DXGI_FORMAT format, std::string destination);
  static void writeTGA(int width, int height, std::vector<uint8_t> &pixelData,
                       std::string destination);
  static std::vector<uint8_t> readTGA(std::string destination);
};
