#include "generic/Textures.h"

#define HRESULT_E_NOT_SUPPORTED static_cast<HRESULT>(0x80070032L)
using namespace DirectX;
namespace Scenario::Gfx::Textures {
void writeDDS(const int width, const int height,
              std::vector<uint8_t> &pixelData, const DXGI_FORMAT format,
              const std::string &path) {
  auto wPath{std::wstring(path.begin(), path.end())};
  Image image(width, height, format, sizeof(uint8_t) * width * 4,
              sizeof(uint8_t) * width * height, pixelData.data());
  SaveToDDSFile(image, DDS_FLAGS_NONE, wPath.c_str());
}

void writeMipMapDDS(const int width, const int height,
                    std::vector<uint8_t> &pixelData, const DXGI_FORMAT format,
                    const std::string &path, bool compress) {
  Image image(width, height, format, sizeof(uint8_t) * width * 4,
              sizeof(uint8_t) * width * height, pixelData.data());
  ScratchImage uncompressed;
  uncompressed.InitializeFromImage(image);
  ScratchImage mipChain;
  GenerateMipMaps(uncompressed.GetImages(), 1, uncompressed.GetMetadata(),
                  TEX_FILTER_FLAGS::TEX_FILTER_FORCE_NON_WIC, 0, mipChain);
  auto wPath{std::wstring(path.begin(), path.end())};
  if (compress) {
    ScratchImage compressedImage;
    auto hr = Compress(mipChain.GetImages(), mipChain.GetImageCount(),
                       mipChain.GetMetadata(), DXGI_FORMAT_BC1_UNORM,
                       TEX_COMPRESS_FLAGS::TEX_COMPRESS_DEFAULT,
                       TEX_THRESHOLD_DEFAULT, compressedImage);

    SaveToDDSFile(compressedImage.GetImages(), compressedImage.GetImageCount(),
                  compressedImage.GetMetadata(), DDS_FLAGS::DDS_FLAGS_NONE,
                  wPath.c_str());
  } else {
    SaveToDDSFile(mipChain.GetImages(), mipChain.GetImageCount(),
                  mipChain.GetMetadata(), DDS_FLAGS::DDS_FLAGS_NONE,
                  wPath.c_str());
  }
}

void writeTGA(const int width, const int height, std::vector<uint8_t> pixelData,
              const std::string &path) {
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
} // namespace Scenario::Gfx::Textures