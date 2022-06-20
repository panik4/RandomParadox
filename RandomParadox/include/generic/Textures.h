#pragma once
#include "DirectXTex.h"
#include "FastWorldGenerator.h"

namespace Scenario::Gfx::Textures {
void writeDDS(const int width, const int height,
              std::vector<uint8_t> &pixelData, const DXGI_FORMAT format,
              const std::string &path);
void writeTGA(const int width, const int height,
              std::vector<uint8_t> &pixelData, const std::string &path);
std::vector<uint8_t> readTGA(const std::string &path);
std::vector<uint8_t> readDDS(const std::string &path);
}; // namespace Gfx::Textures
