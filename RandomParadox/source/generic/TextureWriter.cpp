#include "generic/TextureWriter.h"

void TextureWriter::writeDDS(int width, int height,
                             std::vector<uint8_t> &pixelData,
                             DXGI_FORMAT format, std::string destination) {
  std::wstring destinationPath =
      std::wstring(destination.begin(), destination.end());
  Image image = Image();
  image.width = width;
  image.height = height;
  image.format = format;
  image.rowPitch = sizeof(uint8_t) * image.width * 4;
  image.slicePitch = sizeof(uint8_t) * image.width * image.height;
  image.pixels = pixelData.data();
  SaveToDDSFile(image, DDS_FLAGS_NONE, destinationPath.c_str());
}

void TextureWriter::writeTGA(int width, int height,
                             std::vector<uint8_t> &pixelData,
                             std::string destination) {
  std::wstring destinationPath =
      std::wstring(destination.begin(), destination.end());
  Image image = Image();
  image.width = width;
  image.height = height;
  image.format = DXGI_FORMAT_B8G8R8A8_UNORM;
  image.rowPitch = sizeof(uint8_t) * image.width * 4;
  image.slicePitch = sizeof(uint8_t) * image.width * image.height;
  image.pixels = pixelData.data();
  SaveToTGAFile(image, TGA_FLAGS_NONE, destinationPath.c_str());
}

std::vector<uint8_t> TextureWriter::readTGA(std::string destination) {
  std::wstring destinationPath =
      std::wstring(destination.begin(), destination.end());
  ScratchImage image;
  DirectX::LoadFromTGAFile(destinationPath.c_str(), nullptr, image);
  std::vector<uint8_t> pixelData;
  for (int i = 0; i < image.GetImages()->height * image.GetImages()->width * 4;
       i++) {
    pixelData.push_back(image.GetImages()->pixels[i]);
  }
  for (int i = 0; i < pixelData.size(); i += 4)
    std::swap(pixelData[i], pixelData[i + 2]);
  return pixelData;
}
