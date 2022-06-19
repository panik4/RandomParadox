#include "generic/Textures.h"

namespace Graphics::Textures {
void writeDDS(const int width, const int height,
              std::vector<uint8_t> &pixelData, const DXGI_FORMAT format,
              const std::string &path) {
  auto wPath{std::wstring(path.begin(), path.end())};
  Image image(width, height, format, sizeof(uint8_t) * width * 4,
              sizeof(uint8_t) * width * height, pixelData.data());
  SaveToDDSFile(image, DDS_FLAGS_NONE, wPath.c_str());
}

void writeTGA(const int width, const int height,
              std::vector<uint8_t> &pixelData, const std::string &path) {
  auto wPath{std::wstring(path.begin(), path.end())};
  Image image(width, height, DXGI_FORMAT_B8G8R8A8_UNORM,
              sizeof(uint8_t) * width * 4, sizeof(uint8_t) * width * height,
              pixelData.data());
  SaveToTGAFile(image, TGA_FLAGS_NONE, wPath.c_str());
}

std::vector<uint8_t> readTGA(const std::string &path) {
  auto wPath{std::wstring(path.begin(), path.end())};
  ScratchImage image;
  DirectX::LoadFromTGAFile(wPath.c_str(), nullptr, image);
  std::vector<uint8_t> pixelData;
  for (int i = 0; i < image.GetImages()->height * image.GetImages()->width * 4;
       i++) {
    pixelData.push_back(image.GetImages()->pixels[i]);
  }
  for (int i = 0; i < pixelData.size(); i += 4)
    std::swap(pixelData[i], pixelData[i + 2]);
  return pixelData;
}

std::vector<uint8_t> readDDS(const std::string &path) {
  auto wPath{std::wstring(path.begin(), path.end())};
  ScratchImage image;
  DirectX::LoadFromDDSFile(wPath.c_str(), DDS_FLAGS_FORCE_RGB, nullptr, image);
  ScratchImage destImage;
  auto hr =
      Decompress(image.GetImages(), image.GetImageCount(), image.GetMetadata(),
                 DXGI_FORMAT_B8G8R8A8_UNORM, destImage);
  std::vector<uint8_t> pixelData;
  if (!hr) {
    for (int i = 0;
         i < destImage.GetImages()->height * destImage.GetImages()->width * 4;
         i++) {
      pixelData.push_back(destImage.GetImages()->pixels[i]);
    }
  } else {
    for (int i = 0;
         i < image.GetImages()->height * image.GetImages()->width * 4; i++) {
      pixelData.push_back(image.GetImages()->pixels[i]);
    }
  }
  return pixelData;
}
} // namespace Graphics::Textures