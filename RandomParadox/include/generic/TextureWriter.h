#pragma once
#include "FastWorldGenerator.h"
#include "DirectXTex.h"

using namespace DirectX;
class TextureWriter {
public:
  static void writeDDS(const int width, const int height,
                       std::vector<uint8_t> &pixelData,
                       const DXGI_FORMAT format, const std::string& path);
  static void writeTGA(const int width, const int height,
                       std::vector<uint8_t> &pixelData,
                       const std::string& path);
  static std::vector<uint8_t> readTGA(const std::string &path);
  static std::vector<uint8_t> readDDS(const std::string &path);
};
