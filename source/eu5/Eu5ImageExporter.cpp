#include "eu5/Eu5ImageExporter.h"
namespace Rpx::Gfx::Eu5 {
using namespace Arda::Gfx::Textures;
using namespace Fwg;
using namespace Fwg::Gfx;

ImageExporter::ImageExporter() {}

ImageExporter::ImageExporter(const std::string &gamePath,
                             const std::string &gameTag)
    : Rpx::Gfx::ImageExporter(gamePath, gameTag) {}

ImageExporter::~ImageExporter() {}

std::vector<std::vector<float>>
splitIntoTiles(const std::vector<float> &imageData, int width, int height,
               int tilesX, int tilesY) {
  if (tilesX <= 0 || tilesY <= 0) {
    throw std::runtime_error("tilesX and tilesY must be > 0");
  }

  int tileW = width / tilesX;
  int tileH = height / tilesY;

  // Pre-allocate output container
  std::vector<std::vector<float>> tiles(tilesX * tilesY);

  // Reverse tile rows (top-to-bottom flip), match your original logic
  for (int ty = tilesY - 1; ty >= 0; --ty) {
    for (int tx = 0; tx < tilesX; ++tx) {

      // Same indexing scheme as your original:
      // (tilesY - 1 - ty) = flipped vertical index
      std::vector<float> &tile = tiles[(tilesY - 1 - ty) + tx * tilesY];

      tile.reserve(tileW * tileH);

      for (int y = 0; y < tileH; ++y) {
        int srcY = ty * tileH + y;
        if (srcY >= height)
          srcY = height - 1; // safety clamp

        for (int x = 0; x < tileW; ++x) {
          int srcX = tx * tileW + x;
          if (srcX >= width)
            srcX = width - 1; // safety clamp

          tile.push_back(imageData[srcY * width + srcX]);
        }
      }
    }
  }

  return tiles;
}

std::vector<std::vector<std::vector<float>>>
splitIntoTiles2D(const std::vector<float> &imageData, int width, int height,
                 int tilesX, int tilesY) {
  if (tilesX <= 0 || tilesY <= 0) {
    throw std::runtime_error("tilesX and tilesY must be > 0");
  }

  int tileW = width / tilesX;
  int tileH = height / tilesY;

  // Pre-allocate 2D vector: rows × columns
  std::vector<std::vector<std::vector<float>>> tiles2D(
      tilesY, std::vector<std::vector<float>>(tilesX));

  // Reverse tile rows (top-to-bottom flip)
  for (int ty = tilesY - 1; ty >= 0; --ty) {
    for (int tx = 0; tx < tilesX; ++tx) {

      std::vector<float> &tile = tiles2D[tilesY - 1 - ty][tx];
      tile.reserve(tileW * tileH);

      for (int y = 0; y < tileH; ++y) {
        int srcY = ty * tileH + y;
        if (srcY >= height)
          srcY = height - 1;

        for (int x = 0; x < tileW; ++x) {
          int srcX = tx * tileW + x;
          if (srcX >= width)
            srcX = width - 1;

          tile.push_back(imageData[srcY * width + srcX]);
        }
      }
    }
  }

  return tiles2D;
}

std::vector<std::vector<float>>
extract128Tiles(const std::vector<float> &imageData, int width, int height) {
  constexpr int TILE = 128;

  // Number of tiles per dimension (partial tiles included)
  int tilesX = (width + TILE - 1) / TILE;
  int tilesY = (height + TILE - 1) / TILE;

  // Allocate output: row-major, bottom-to-top, left-to-right
  std::vector<std::vector<float>> tiles;
  tiles.resize(tilesX * tilesY);

  for (int ty = 0; ty < tilesY; ++ty) {
    for (int tx = 0; tx < tilesX; ++tx) {

      // Quadtree-friendly index: bottom row first
      // bottom tile row = ty=0
      int qy = tilesY - 1 - ty;     // invert Y so bottom-to-top
      int index = qy * tilesX + tx; // row-major

      std::vector<float> &tile = tiles[index];
      tile.reserve(TILE * TILE);

      // Compute pixel bounds of this tile
      int x0 = tx * TILE;
      int y0 = ty * TILE;

      int w = std::min<int>(TILE, width - x0);
      int h = std::min<int>(TILE, height - y0);

      // Copy pixels
      for (int y = 0; y < h; ++y) {
        int srcY = y0 + y;
        for (int x = 0; x < w; ++x) {
          int srcX = x0 + x;
          tile.push_back(imageData[srcY * width + srcX]);
        }
      }
    }
  }

  return tiles;
}

// Child tiles are all 128x128 floats
std::vector<float> downsampleTile4To1(const std::vector<float> &c00,
                                      const std::vector<float> &c10,
                                      const std::vector<float> &c01,
                                      const std::vector<float> &c11) {
  constexpr int TILE = 128;
  std::vector<float> out(TILE * TILE);

  for (int y = 0; y < TILE; ++y) {
    for (int x = 0; x < TILE; ++x) {

      // Source location in child tiles (2x resolution)
      int cx = x * 2;
      int cy = y * 2;

      float sum = 0.f;

      // Child 00
      sum += c00[(cy + 0) * (TILE * 2) + (cx + 0)];
      sum += c00[(cy + 0) * (TILE * 2) + (cx + 1)];
      sum += c00[(cy + 1) * (TILE * 2) + (cx + 0)];
      sum += c00[(cy + 1) * (TILE * 2) + (cx + 1)];

      // Child 10
      sum += c10[(cy + 0) * (TILE * 2) + (cx + 0)];
      sum += c10[(cy + 0) * (TILE * 2) + (cx + 1)];
      sum += c10[(cy + 1) * (TILE * 2) + (cx + 0)];
      sum += c10[(cy + 1) * (TILE * 2) + (cx + 1)];

      // Child 01
      sum += c01[(cy + 0) * (TILE * 2) + (cx + 0)];
      sum += c01[(cy + 0) * (TILE * 2) + (cx + 1)];
      sum += c01[(cy + 1) * (TILE * 2) + (cx + 0)];
      sum += c01[(cy + 1) * (TILE * 2) + (cx + 1)];

      // Child 11
      sum += c11[(cy + 0) * (TILE * 2) + (cx + 0)];
      sum += c11[(cy + 0) * (TILE * 2) + (cx + 1)];
      sum += c11[(cy + 1) * (TILE * 2) + (cx + 0)];
      sum += c11[(cy + 1) * (TILE * 2) + (cx + 1)];

      out[y * TILE + x] = sum * 0.0625f; // average 16 samples
    }
  }

  return out;
}

void writeMipmapLevels(const std::vector<float> &fullImage, int width,
                       int height) {
  constexpr int TILE = 128;

  // Level 0 tiles
  std::vector<std::vector<float>> current =
      extract128Tiles(fullImage, width, height);

  int tilesX = (width + TILE - 1) / TILE;
  int tilesY = (height + TILE - 1) / TILE;

  int level = 0;

  while (true) {

    // Create output folder
    std::string levelDir = Cfg::Values().mapsPath + "//debugTerrain//level_" +
                           std::to_string(level);

    std::filesystem::create_directories(levelDir);

    // ---- Write all tiles for this level ----
    int num = 0;
    for (const auto &tile : current) {
      if (level < 3) {
        continue;
      }
      std::string name = "heightmap_" + std::to_string(num) + ".png";
      std::string path = levelDir + "//" + name;

      Fwg::Utils::Logging::logLine("Saving L" + std::to_string(level) +
                                   " tile: " + path);

      Fwg::Gfx::Png::save(tile, TILE, TILE, path, true, LCT_GREY, 16);

      num++;
    }

    // If only 1 tile remains, we are done
    if (tilesX == 1 && tilesY == 1)
      break;

    // ---- Build next level ----
    int nextTilesX = (tilesX + 1) / 2;
    int nextTilesY = (tilesY + 1) / 2;

    std::vector<std::vector<float>> next;
    next.reserve(nextTilesX * nextTilesY);

    for (int ty = 0; ty < nextTilesY; ++ty) {
      for (int tx = 0; tx < nextTilesX; ++tx) {

        // Indices of 4 children
        int c00 = (ty * 2 + 0) * tilesX + (tx * 2 + 0);
        int c10 = (ty * 2 + 0) * tilesX + (tx * 2 + 1);
        int c01 = (ty * 2 + 1) * tilesX + (tx * 2 + 0);
        int c11 = (ty * 2 + 1) * tilesX + (tx * 2 + 1);

        // Clamp for odd tile counts
        auto &t00 = current[c00];
        auto &t10 = (tx * 2 + 1 < tilesX) ? current[c10] : current[c00];
        auto &t01 = (ty * 2 + 1 < tilesY) ? current[c01] : current[c00];
        auto &t11 = (tx * 2 + 1 < tilesX && ty * 2 + 1 < tilesY) ? current[c11]
                                                                 : current[c00];

        next.push_back(downsampleTile4To1(t00, t10, t01, t11));
      }
    }

    // Prepare next iteration
    current = std::move(next);
    tilesX = nextTilesX;
    tilesY = nextTilesY;
    level++;
    if (level > 6)
      break;
  }
}

std::vector<float> pad128to132_strategyA(const std::vector<float> &src) {
  constexpr int SRC = 128;
  constexpr int DST = 132;

  std::vector<float> out(DST * DST);

  for (int y = 0; y < DST; ++y) {
    int sy = std::min<int>(y, SRC - 1);

    for (int x = 0; x < DST; ++x) {
      int sx = std::min<int>(x, SRC - 1);
      out[y * DST + x] = src[sy * SRC + sx];
    }
  }

  return out;
}

Fwg::Gfx::Image ImageExporter::dumpHeightmap(
    const std::vector<float> &heightMap, const std::string &path,
    const std::string &colourMapKey, int exportWidth, int exportHeight) const {
  //  rasterize this image into 16 different images , based on the heightmap,
  //  presenting 16 different chunks of the heightmap
  auto workingWidth = Fwg::Cfg::Values().width;
  auto workingHeight = Fwg::Cfg::Values().height;

  // we take the copy to modify the sealevel and make ti even. Eu5 doesn't care
  // about this detail, and we can save on both memory and disk space
  auto hmapCopy = heightMap;
  // make all pixels below sealevel zero
  for (auto &val : hmapCopy) {
    if (val < static_cast<float>(Fwg::Cfg::Values().seaLevel)) {
      val = 9.0f;
    }
  }
  auto mipmapWorkingWidth = exportWidth / 2;
  auto mipmapWorkingHeight = exportHeight / 2;
  auto scaledHmap = Fwg::Utils::scaleBufferWithInterpolation(
      hmapCopy, workingWidth, workingHeight, mipmapWorkingWidth,
      mipmapWorkingHeight);

  auto tiles =
      splitIntoTiles(scaledHmap, mipmapWorkingWidth, mipmapWorkingHeight, 4, 4);
  int i = 0;
  for (auto &tile : tiles) {
    i++;
    auto scaledTile = Fwg::Utils::scaleBufferWithInterpolation(
        tile, mipmapWorkingWidth / 4, mipmapWorkingHeight / 4, exportWidth / 8,
        exportHeight / 8);
    std::string num = std::to_string(i);
    auto name = "heightmap_" + num + "_16";
    auto hmapPath =
        path + "decals//" + "//" + name + "//" + name + "_height.png";
    Fwg::Utils::Logging::logLine("Saving heightmap tile: " + hmapPath);
    Fwg::Gfx::Png::save(scaledTile, exportWidth / 8, exportHeight / 8, hmapPath,
                        true, LCT_GREY, 16);
  }

  auto initialXTiles = mipmapWorkingWidth / 64;
  auto initialYTiles = mipmapWorkingHeight / 64;
  std::vector<std::vector<std::vector<unsigned char>>> encodedPngs;

  // Constants
  constexpr int baseTileSize = 128;                      // Target tile size
  constexpr int paddedTileSize = 132;                    // Tile + 4px padding
  constexpr int padding = paddedTileSize - baseTileSize; // 4 pixels

  // Container for encoded PNGs per mipmap level
  std::vector<std::vector<std::vector<std::vector<unsigned char>>>>
      mipmapLevels;

  // Start from highest-detail level
  for (int level = 0; level < 9; ++level) {
    auto levelTilesX = initialXTiles / static_cast<int>(pow(2, level));
    auto levelTilesY = initialYTiles / static_cast<int>(pow(2, level));
    auto tileSizeX = mipmapWorkingWidth / levelTilesX;
    auto tileSizeY = mipmapWorkingHeight / levelTilesY;
    Fwg::Utils::Logging::logLine("Mipmap level " + std::to_string(level) +
                                 " TilesX: " + std::to_string(levelTilesX) +
                                 " TilesY: " + std::to_string(levelTilesY));
    Fwg::Utils::Logging::logLine("TileSizeX: " + std::to_string(tileSizeX) +
                                 " TileSizeY: " + std::to_string(tileSizeY));
    auto d2TileGrid =
        splitIntoTiles2D(scaledHmap, mipmapWorkingWidth, mipmapWorkingHeight,
                         levelTilesX, levelTilesY);
    Fwg::Utils::Logging::logLine(
        "Size of d2TileGrid: " + std::to_string(d2TileGrid.size()) + " x " +
        std::to_string(d2TileGrid[0].size()));
    auto inputAreaWidth = mipmapWorkingWidth;
    auto inputAreaHeight = mipmapWorkingHeight;
    Fwg::Utils::Logging::logLine("Processing mipmap level " +
                                 std::to_string(level));

    int rows = levelTilesY;
    int cols = levelTilesX;
    Fwg::Utils::Logging::logLine("Mipmap level " + std::to_string(level) +
                                 " Rows: " + std::to_string(rows) +
                                 " Cols: " + std::to_string(cols));

    // Storage for encoded PNGs
    std::vector<std::vector<std::vector<unsigned char>>> encodedPngs(
        rows, std::vector<std::vector<unsigned char>>(cols));

    // Optional: limit number of concurrent threads
    constexpr int maxThreads = 32;
    std::vector<std::thread> threads;
    threads.reserve(maxThreads);
    constexpr int baseTileSize = 128;
    constexpr int paddedTileSize = 132;
    constexpr int padding = 2;

    for (int r = 0; r < rows; ++r) {
      Fwg::Utils::Logging::logLine("Processing mipmap row " +

                                   std::to_string(r));
      threads.emplace_back([r, cols, rows, &d2TileGrid, &encodedPngs, tileSizeX,
                            tileSizeY]() {
        for (int c = 0; c < cols; ++c) {
          // --------------------------------------------------
          // Resolve source-space coordinates ONCE
          // --------------------------------------------------
          const int srcRow = rows - 1 - r;
          const int srcCol = c;

          const bool hasLeft = (srcCol - 1) >= 0;
          const bool hasRight = (srcCol + 1) < cols;
          const bool hasTop = (srcRow + 1) < rows;
          const bool hasBottom = (srcRow - 1) >= 0;

          // --------------------------------------------------
          // 1. Upscale CENTER tile to 128x128
          // --------------------------------------------------
          auto center = Fwg::Utils::scaleBufferWithInterpolation(
              d2TileGrid[srcRow][srcCol], tileSizeX, tileSizeY, baseTileSize,
              baseTileSize);

          // Allocate padded tile (initialized to black)
          std::vector<float> paddedTile(paddedTileSize * paddedTileSize, 0.0f);

          // --------------------------------------------------
          // CENTER
          // --------------------------------------------------
          for (int y = 0; y < baseTileSize; ++y) {
            for (int x = 0; x < baseTileSize; ++x) {
              paddedTile[(y + padding) * paddedTileSize + (x + padding)] =
                  center[y * baseTileSize + x];
            }
          }

          // --------------------------------------------------
          // LEFT
          // --------------------------------------------------
          if (hasLeft) {
            auto left = Fwg::Utils::scaleBufferWithInterpolation(
                d2TileGrid[srcRow][srcCol - 1], tileSizeX, tileSizeY,
                baseTileSize, baseTileSize);

            for (int y = 0; y < baseTileSize; ++y) {
              for (int x = 0; x < padding; ++x) {
                paddedTile[(y + padding) * paddedTileSize + x] =
                    left[y * baseTileSize + (baseTileSize - padding + x)];
              }
            }
          }

          // --------------------------------------------------
          // RIGHT
          // --------------------------------------------------
          if (hasRight) {
            auto right = Fwg::Utils::scaleBufferWithInterpolation(
                d2TileGrid[srcRow][srcCol + 1], tileSizeX, tileSizeY,
                baseTileSize, baseTileSize);

            for (int y = 0; y < baseTileSize; ++y) {
              for (int x = 0; x < padding; ++x) {
                paddedTile[(y + padding) * paddedTileSize +
                           (padding + baseTileSize + x)] =
                    right[y * baseTileSize + x];
              }
            }
          }

          // --------------------------------------------------
          // TOP (north)
          // --------------------------------------------------
          if (hasTop) {
            auto top = Fwg::Utils::scaleBufferWithInterpolation(
                d2TileGrid[srcRow + 1][srcCol], tileSizeX, tileSizeY,
                baseTileSize, baseTileSize);

            for (int y = 0; y < padding; ++y) {
              for (int x = 0; x < baseTileSize; ++x) {
                paddedTile[y * paddedTileSize + (x + padding)] =
                    top[(baseTileSize - padding + y) * baseTileSize + x];
              }
            }
          }

          // --------------------------------------------------
          // BOTTOM (south)
          // --------------------------------------------------
          if (hasBottom) {
            auto bottom = Fwg::Utils::scaleBufferWithInterpolation(
                d2TileGrid[srcRow - 1][srcCol], tileSizeX, tileSizeY,
                baseTileSize, baseTileSize);

            for (int y = 0; y < padding; ++y) {
              for (int x = 0; x < baseTileSize; ++x) {
                paddedTile[(padding + baseTileSize + y) * paddedTileSize +
                           (x + padding)] = bottom[y * baseTileSize + x];
              }
            }
          }

          // --------------------------------------------------
          // BOTTOM-RIGHT corner
          // --------------------------------------------------
          if (hasBottom && hasRight) {
            auto br = Fwg::Utils::scaleBufferWithInterpolation(
                d2TileGrid[srcRow - 1][srcCol + 1], tileSizeX, tileSizeY,
                baseTileSize, baseTileSize);

            for (int y = 0; y < padding; ++y) {
              for (int x = 0; x < padding; ++x) {
                paddedTile[(padding + baseTileSize + y) * paddedTileSize +
                           (padding + baseTileSize + x)] =
                    br[y * baseTileSize + x];
              }
            }
          }

          // ------------------------------------------------------------
          // 7. Encode
          // ------------------------------------------------------------
          encodedPngs[r][c] = Fwg::Gfx::Png::getEncodedPng(
              paddedTile, paddedTileSize, paddedTileSize, LCT_GREY, 16, 255);
        }
      });

      if (threads.size() >= maxThreads) {
        for (auto &t : threads)
          t.join();
        threads.clear();
      }
    }

    // Join any remaining threads
    for (auto &t : threads)
      t.join();

    // Store this level
    mipmapLevels.push_back(encodedPngs);
  }
  // write all the encoded pngs to a .bin file for inclusion in the mod
  std::string outInfoPath = path + "//terrain_cache//heightmap.info";
  std::string outBinPath = path + "//terrain_cache//heightmap.bin";
  std::ofstream outBin(outBinPath, std::ios::binary);
  auto outInfoTemplate =
      Fwg::Parsing::readFile(Cfg::Values().resourcePath +
                             "eu5/gfx/terrain2/terrain_cache/heightmap.info");
  std::string outInfo = "";

  if (!outBin) {
    throw std::runtime_error("Failed to open output file: " + outBinPath);
  }
  int offset = 0;
  for (size_t lvl = 0; lvl < mipmapLevels.size(); ++lvl) {
    const auto &levelTiles = mipmapLevels[lvl];
    int rows = static_cast<int>(levelTiles.size());
    int cols = static_cast<int>(levelTiles[0].size());
    Fwg::Utils::Logging::logLine("Writing mipmap level " + std::to_string(lvl) +
                                 " Rows: " + std::to_string(rows) +
                                 " Cols: " + std::to_string(cols));
    for (int r = 0; r < rows; ++r) {
      for (int c = 0; c < cols; ++c) {
        const auto &encodedPng = levelTiles[r][c];
        // Write size of PNG data
        uint32_t dataSize = static_cast<uint32_t>(encodedPng.size());
        //  Write PNG data
        outBin.write(reinterpret_cast<const char *>(encodedPng.data()),
                     dataSize);
        // Write info log in the format:
        outInfo.append("{ offset=" + std::to_string(offset) +
                       " size=" + std::to_string(dataSize) + "}\n");
        offset += dataSize;
      }
    }
  }
  outBin.close();
  Fwg::Parsing::replaceOccurence(outInfoTemplate, "templateTileInfos", outInfo);
  Fwg::Utils::Logging::logLine("Writing heightmap info file: " + outInfoPath);
  Fwg::Parsing::writeFile(outInfoPath, outInfoTemplate);

  return Fwg::Gfx::Image(0, 0, 24);
}

Fwg::Gfx::Image ImageExporter::dumpDecalMasks(
    const Fwg::Terrain::TerrainData &terrainData,
    const Fwg::Climate::ClimateData &climateData, const std::string &path,
    const std::string &colourMapKey, int exportWidth, int exportHeight) const {
  auto imageHumidity = climateData.humidities;
  Fwg::Utils::normalizeVector(imageHumidity, 0.0f, 255.0f);
  auto scaledHeightmap = Fwg::Utils::scaleBufferWithInterpolation(
      imageHumidity, Fwg::Cfg::Values().width, Fwg::Cfg::Values().height,
      exportWidth, exportHeight);
  FastNoiseLite noiseGenerator;
  noiseGenerator.SetSeed(0);
  noiseGenerator.SetFrequency(0.005);
  noiseGenerator.SetFractalOctaves(11);
  noiseGenerator.SetFractalGain(0.7f);
  noiseGenerator.SetNoiseType(
      FastNoiseLite::NoiseType_ValueCubic); // Set the desired noise type
  noiseGenerator.SetFractalType(FastNoiseLite::FractalType_FBm);
  // randomness for trees
  auto noiseMap = Noise::genNoise(noiseGenerator, exportWidth / 4,
                                  exportHeight / 4, 0.0, 0.0, 0, 255, 0.0);

  // we want 

  // rasterize this image into 16 different images , based on the heightmap,
  // presenting 16 different chunks of the heightmap
  auto tiles = splitIntoTiles(scaledHeightmap, exportWidth, exportHeight, 4, 4);
  int i = 0;
  for (auto &tile : tiles) {
    i++;
    // for now write no ground textures, keep it blank

    std::string num = std::to_string(i);
    auto folderName = "heightmap_" + num + "_16";
    for (int x = 0; x < 16; x++) {
      auto name = folderName + "_mask_" + std::format("{:02}", x) + ".png";
      auto hmapPath = path + "//" + folderName + "//" + name;
      Fwg::Utils::Logging::logLine("Saving mask tile: " + hmapPath);
      // we want grayscale 8bit non-linear integer
      Fwg::Gfx::Png::save(tile, exportWidth / 4, exportHeight / 4, hmapPath,
                          true, LCT_GREY);
    }
    Fwg::Utils::Logging::logLine("Saving heightmap bitmask tile: " +
                                 (path + "//" + folderName + "//heightmap_" +
                                  std::to_string(i) + "_16_bitmask.png"));

    // also write the heightmap bitmask
    Fwg::Gfx::Png::save(noiseMap, exportWidth / 4, exportHeight / 4,
                        path + "//" + folderName + "//heightmap_" +
                            std::to_string(i) + "_16_bitmask.png",
                        true, LCT_GREY);
  }

  return Fwg::Gfx::Image(0, 0, 24);
}

Fwg::Gfx::Image ImageExporter::dumpTerrainMasks(
    const Fwg::Terrain::TerrainData &terrainData,
    const Fwg::Climate::ClimateData &climateData, const std::string &path,
    const std::string &colourMapKey, int exportWidth, int exportHeight) const {

  std::map<int, std::string> nameMapping{
      {0, "asia_savannah_base_mask"},
      {1, "asian_farms_mask"},
      {2, "conifer_forest_mask"},
      {3, "continental_grass_dirt_mask"},
      {4, "continental_hills_mask"},
      {5, "coral_green_mask"},
      {6, "coral_mask"},
      {7, "coral_variation_mask"},
      {8, "dirt_01_mask"},
      {9, "elephant_mask"},
      {10, "estepa_base_01_mask"},
      {11, "estepa_variation_01_mask"},
      {12, "farms_01_mask"},
      {13, "farms_02_mask"},
      {14, "forest_01_mask"},
      {15, "hills_base_01_mask"},
      {16, "impassable_desert_canyon_mask"},
      {17, "impassable_desert_variation_01_mask"},
      {18, "impassable_desert_variation_02_mask"},
      {19, "iron_mask"},
      {20, "jungle_variation_soil_01_mask"},
      {21, "marsh_base_01_mask"},
      {22, "marsh_variation_02_mask"},
      {23, "mask_devastation_mud_01"},
      {24, "med_drylands_02_mask"},
      {25, "med_drylands_variation_mask"},
      {26, "mountain_artic_variation_01_mask"},
      {27, "mountain_med_variation_mask"},
      {28, "mountains_gravel_mask"},
      {29, "mountains_variation_dirt_02_mask"},
      {30, "mountains_variation_rock_01_mask"},
      {31, "mud_base_01_mask"},
      {32, "plains_01_mask"},
      {33, "plains_02_mask"},
      {34, "plains_mediterranean_mask"},
      {35, "plains_variation_grass_01_mask"},
      {36, "plains_variation_grass_02_mask"},
      {37, "plains_variation_grass_03_mask"},
      {38, "rock_01_mask"},
      {39, "sand_01_mask"},
      {40, "sand_02_mask"},
      {41, "sand_dunes_01_mask"},
      {42, "sand_dunes_02_mask"},
      {43, "savannah_base_01_mask"},
      {44, "savannah_variation_grass_01_mask"},
      {45, "snow_mask"},
      {46, "tundra_base_01_mask"},
      {47, "tundra_variation_grass_01_mask"},
      {48, "tundra_variation_rock_02_mask"},
      {49, "water_mask"},
  };

  auto imageHumidity = climateData.humidities;
  Fwg::Utils::normalizeVector(imageHumidity, 0.0f, 255.0f);
  for (int i = 0; i < 50; i++) {
    FastNoiseLite noiseGenerator;
    noiseGenerator.SetSeed(i);
    noiseGenerator.SetFrequency(0.05);
    noiseGenerator.SetFractalOctaves(11);
    noiseGenerator.SetFractalGain(0.7f);
    noiseGenerator.SetNoiseType(
        FastNoiseLite::NoiseType_ValueCubic); // Set the desired noise type
    noiseGenerator.SetFractalType(FastNoiseLite::FractalType_FBm);
    // randomness for trees
    auto treeNoiseMap =
        Noise::genNoise(noiseGenerator, Fwg::Cfg::Values().width,
                        Fwg::Cfg::Values().height, 0.0, 0.0, 0, 255, 0.0);
    auto scaledHeightmap = Fwg::Utils::scaleBufferWithInterpolation(
        treeNoiseMap, Fwg::Cfg::Values().width, Fwg::Cfg::Values().height,
        exportWidth, exportHeight);
    Fwg::Gfx::Png::save(
        Fwg::Gfx::Image(exportWidth, exportHeight, 24, scaledHeightmap),
        path + "//" + nameMapping.at(i) + ".png", true, LCT_GREY);
  }

  return Fwg::Gfx::Image(0, 0, 24);
}

void ImageExporter::Eu5ColourMaps(
    const Fwg::Terrain::TerrainData &terrainData,
    const Fwg::Climate::ClimateData &climateData,
    const Arda::Civilization::CivilizationLayer &civLayer,
    const std::string &path, int exportWidth, int exportHeight) {
  Fwg::Utils::Logging::logLine("Eu5 Format Converter: Writing colour maps");

  auto &config = Cfg::Values();
  auto altitude = terrainData.detailedHeightMap;
  auto scaledHeightmap = Fwg::Utils::scaleBufferWithInterpolation(
      altitude, Fwg::Cfg::Values().width, Fwg::Cfg::Values().height,
      exportWidth, exportHeight);
  int factor = 1;

  std::vector<uint8_t> pixels(exportWidth * exportHeight * 4, 0);
  Utils::Logging::logLine(
      "ImageExporter::Writing watercolor_rgb_waterspec_a to ", path);
  using namespace DirectX;

  for (auto h = 0; h < exportHeight; h++) {
    for (auto w = 0; w < exportWidth; w++) {
      auto referenceIndex = h * exportWidth + w;
      double depth = (double)scaledHeightmap[referenceIndex] /
                     (double)Cfg::Values().seaLevel;
      auto imageIndex =
          exportHeight * exportWidth - (h * exportWidth + (exportWidth - w));
      imageIndex *= 4;
      if (depth < 1.0) {
        writeBufferPixels(
            pixels, imageIndex,
            Fwg::Gfx::Colour(16.0 * depth, 24.0 * depth, 49.0 * depth), 255);
      } else {
        writeBufferPixels(pixels, imageIndex, Fwg::Gfx::Colour(50, 100, 100),
                          255);
      }
    }
  }
  Arda::Gfx::Textures::writeMipMapDDS(
      exportWidth, exportHeight, pixels, DXGI_FORMAT_B8G8R8A8_UNORM,
      path + "//water//watercolor_rgb_waterspec_a.dds", true);
  // std::fill(pixels.begin(), pixels.end(), 0);
  // Arda::Gfx::Textures::writeMipMapDDS(exportWidth / 4, exportHeight / 4,
  // pixels,
  //                                     DXGI_FORMAT_B8G8R8A8_UNORM,
  //                                     path + "//water//foam_map.dds");
  // Arda::Gfx::Textures::writeMipMapDDS(exportWidth / 8, exportHeight / 8,
  // pixels,
  //                                     DXGI_FORMAT_B8G8R8A8_UNORM,
  //                                     path + "//water//flowmap.dds");
}

void ImageExporter::mapObjectMasks(
    const Fwg::Terrain::TerrainData &terrainData,
    const Fwg::Climate::ClimateData &climateData,
    const Arda::Civilization::CivilizationLayer &civLayer,
    const std::string &path, int exportWidth, int exportHeight) {
  auto width = Cfg::Values().width;
  auto height = Cfg::Values().height;

  std::vector<std::string> emptyMasks = {
      "decals_mask",
      "dirt_mask",
      "grassmesh_mask",
      "reeds_mask",
      "rock_mask",
      "vegetation_forest_japan_mask",
      "vegetation_woods_japan_mask",
  };
  std::shared_ptr<Fwg::Gfx::Image> denseJungleMask =
      std::make_shared<Fwg::Gfx::Image>(width, height, 24);
  std::shared_ptr<Fwg::Gfx::Image> jungleMask =
      std::make_shared<Fwg::Gfx::Image>(width, height, 24);

  std::shared_ptr<Fwg::Gfx::Image> pineMask =
      std::make_shared<Fwg::Gfx::Image>(width, height, 24);

  std::shared_ptr<Fwg::Gfx::Image> forestContinentalMask =
      std::make_shared<Fwg::Gfx::Image>(width, height, 24);
  std::shared_ptr<Fwg::Gfx::Image> woodsContinentalMask =
      std::make_shared<Fwg::Gfx::Image>(width, height, 24);

  for (int i = 0; i < climateData.forestTypes.size(); i++) {
    auto treeCoverType = climateData.forestTypes[i];
    auto forestDensity =
        static_cast<float>(climateData.forestDensity[i]) / 255.0f;
    switch (treeCoverType) {
    case Fwg::Climate::Detail::ForestType::TROPICALMOIST: {
      denseJungleMask->setColourAtIndex(i, 255.0f * forestDensity);
      break;
    }
    case Fwg::Climate::Detail::ForestType::TROPICALDRY: {
      jungleMask->setColourAtIndex(i, 255.0f * forestDensity);
      break;
    }
    case Fwg::Climate::Detail::ForestType::BOREAL: {
      pineMask->setColourAtIndex(i, 255.0f * forestDensity);
      break;
    }
    case Fwg::Climate::Detail::ForestType::TEMPERATEMIXED: {
      if (forestDensity > 0.5f) {
        forestContinentalMask->setColourAtIndex(i, 255.0f * forestDensity);
      } else {
        woodsContinentalMask->setColourAtIndex(i, 255.0f * forestDensity);
      }
      break;
    }
    case Fwg::Climate::Detail::ForestType::TEMPERATENEEDLE: {
      pineMask->setColourAtIndex(i, 255.0f * forestDensity);
      break;
    }
    default:
      break;
    }
  }
  Fwg::Gfx::Png::save(
      Fwg::Gfx::Util::scale(*denseJungleMask, exportWidth, exportHeight, false),
      path + "//jungle_dense_mask.png", false, LCT_GREY);
  Fwg::Gfx::Png::save(
      Fwg::Gfx::Util::scale(*jungleMask, exportWidth, exportHeight, false),
      path + "//jungle_mask.png", false, LCT_GREY);
  Fwg::Gfx::Png::save(
      Fwg::Gfx::Util::scale(*pineMask, exportWidth, exportHeight, false),
      path + "//pine_mask.png", false, LCT_GREY);
  Fwg::Gfx::Png::save(Fwg::Gfx::Util::scale(*forestContinentalMask, exportWidth,
                                           exportHeight, false),
                      path + "//vegetation_forest_continental_oceanic_mask.png",
                      false, LCT_GREY);
  Fwg::Gfx::Png::save(Fwg::Gfx::Util::scale(*woodsContinentalMask, exportWidth,
                                           exportHeight, false),
                      path + "//vegetation_woods_continental_oceanic_mask.png",
                      false, LCT_GREY);

  auto blankImage = Fwg::Gfx::Image(exportWidth, exportHeight, 24);
  blankImage.setColourAtIndex(0, 255);
  for (auto &maskName : emptyMasks) {
    Fwg::Gfx::Png::save(blankImage, path + "//" + maskName + ".png", true,
                        LCT_GREY);
  }
}

void ImageExporter::writeLocations(
    const Fwg::Gfx::Image &provinceMap,
    const std::vector<std::shared_ptr<Arda::ArdaProvince>> &provinces,
    const std::vector<std::shared_ptr<Arda::ArdaRegion>> &regions,
    const std::vector<std::shared_ptr<Arda::ArdaContinent>> &continents,
    const Arda::Civilization::CivilizationLayer &civLayer,
    const std::string &path, int exportWidth, int exportHeight) const {

  Fwg::Utils::Logging::logLine("Eu5 Format Converter: Writing locations map");

  // flatland, hills, mountain_wasteland, mountains, wetlands, lakes,
  // high_lakes, plateau, narrows, coastal_ocean, inland_sea, ocean, deep_ocean,
  // dune_wasteland, mesa_wasteland
  std::map<Fwg::Terrain::LandformId, std::string> elevationMapping = {
      {Fwg::Terrain::LandformId::OCEAN, "ocean"},
      {Fwg::Terrain::LandformId::DEEPOCEAN, "deep_ocean"},
      {Fwg::Terrain::LandformId::PLAINS, "flatland"},
      {Fwg::Terrain::LandformId::LOWHILLS, "hills"},
      {Fwg::Terrain::LandformId::HILLS, "hills"},
      {Fwg::Terrain::LandformId::MOUNTAINS, "mountains"},
      {Fwg::Terrain::LandformId::PEAKS, "mountains"},
      {Fwg::Terrain::LandformId::STEEPPEAKS, "mountains"},
      {Fwg::Terrain::LandformId::CLIFF, "mountains"},
      {Fwg::Terrain::LandformId::HIGHLANDS, "plateau"},
      {Fwg::Terrain::LandformId::VALLEY, "hills"},
      {Fwg::Terrain::LandformId::LAKE, "lakes"},
  };
  enum class ClimateClassId : unsigned char {
    TROPICSRAINFOREST,
    TROPICSMONSOON,
    TROPICSSAVANNA,
    DESERT,
    COLDDESERT,
    HOTSEMIARID,
    COLDSEMIARID,
    TEMPERATEHOT,
    TEMPERATEWARM,
    TEMPERATECOLD,
    CONTINENTALHOT,
    CONTINENTALWARM,
    CONTINENTALCOLD,
    POLARTUNDRA,
    POLARARCTIC,
    SNOW,
    WATER
  };
  // continental, arctic, oceanic, mediterranean, subtropical, tropical,
  // cold_arid, arid,
  std::map<Fwg::Climate::Detail::ClimateClassId, std::string> climateMapping = {
      {Fwg::Climate::Detail::ClimateClassId::TROPICSRAINFOREST, "tropical"},
      {Fwg::Climate::Detail::ClimateClassId::TROPICSMONSOON, "tropical"},
      {Fwg::Climate::Detail::ClimateClassId::TROPICSSAVANNA, "subtropical"},
      {Fwg::Climate::Detail::ClimateClassId::DESERT, "arid"},
      {Fwg::Climate::Detail::ClimateClassId::COLDDESERT, "cold_arid"},
      {Fwg::Climate::Detail::ClimateClassId::HOTSEMIARID, "arid"},
      {Fwg::Climate::Detail::ClimateClassId::COLDSEMIARID, "cold_arid"},
      {Fwg::Climate::Detail::ClimateClassId::TEMPERATEHOT, "mediterranean"},
      {Fwg::Climate::Detail::ClimateClassId::TEMPERATEWARM, "mediterranean"},
      {Fwg::Climate::Detail::ClimateClassId::TEMPERATECOLD, "continental"},
      {Fwg::Climate::Detail::ClimateClassId::CONTINENTALHOT, "continental"},
      {Fwg::Climate::Detail::ClimateClassId::CONTINENTALWARM, "continental"},
      {Fwg::Climate::Detail::ClimateClassId::CONTINENTALCOLD, "continental"},
      {Fwg::Climate::Detail::ClimateClassId::POLARTUNDRA, "continental"},
      {Fwg::Climate::Detail::ClimateClassId::POLARARCTIC, "arctic"},
      {Fwg::Climate::Detail::ClimateClassId::SNOW, "arctic"},
      {Fwg::Climate::Detail::ClimateClassId::WATER, "ocean"},
  };

  auto scaledProvinces =
      Fwg::Gfx::Util::scale(provinceMap, exportWidth, exportHeight, false);
  Fwg::Gfx::Png::save(scaledProvinces, path + "//locations.png", true);

  // vegetation: grassland, farmland, woods, forest, sparse, desert, jungle,

  // now lets write the hex codes of all the locations to the
  // named_locations/00_default.txt file
  std::string fileData = "";
  std::string locationTemplates = "";
  int id = 0;
  std::string lakeText = "{ topography = lakes climate = continental}";
  std::string landText =
      "{ topography = templateTopography vegetation = sparse climate = "
      "templateClimate "
      "religion = catholic culture = swedish raw_material = lumber}";
  std::string coastalText =
      "{ topography = templateTopography vegetation = sparse climate = "
      "templateClimate "
      "religion = catholic culture = swedish raw_material = fish "
      "natural_harbor_suitability = 0.00}";
  std::string oceanText = "{ topography = coastal_ocean climate = arctic}";

  std::string oceanZones = "";
  std::string lakeZones = "";

  for (auto &province : provinces) {
    auto topography = province->dominantLandformId;
    auto climateType = province->climateType;
    auto provName = "prov_" + std::to_string(province->ID);
    fileData.append(provName + " = " + province->toHexString(false, false) +
                    "\n");
    locationTemplates.append(provName + " = ");
    if (province->isSea()) {
      locationTemplates.append(oceanText + "\n");
      oceanZones.append(provName + "\n");
    } else if (province->isLake()) {
      locationTemplates.append(lakeText + "\n");
      lakeZones.append(provName + "\n");
    } else if (province->coastal) {
      locationTemplates.append(coastalText + "\n");
    } else {
      locationTemplates.append(landText + "\n");
    }
    Fwg::Parsing::replaceOccurences(locationTemplates, "templateTopography",
                                    elevationMapping[topography]);
    Fwg::Parsing::replaceOccurences(locationTemplates, "templateClimate",
                                    climateMapping[climateType]);
  }

  Fwg::Parsing::writeFile(path + "//named_locations/00_default.txt", fileData,
                          true);

  Fwg::Parsing::writeFile(path + "//definitions.txt", "");

  Fwg::Parsing::writeFile(path + "//location_templates.txt", locationTemplates);

  // lets try port
  // for every coastal province, we write a port entry, which is any
  // neighbouring province that is sea
  std::string oceanPortTemplate = "LandProvince;SeaZone;x;y;\n";
  for (auto &province : provinces) {
    if (!province->coastal) {
      continue;
    }
    auto coastalProvName = "prov_" + std::to_string(province->ID);
    std::string targetName = "";
    for (auto &neighbourRelation : province->neighbourRelations) {
      if (neighbourRelation->neighbour->isSea()) {
        // we have a coastal province with a sea neighbour, write port entry
        targetName = "prov_" + std::to_string(neighbourRelation->neighbour->ID);
      }
    }
    auto x = province->position.widthCenter;
    auto y = province->position.heightCenter;
    auto assembledLine = coastalProvName + ";" + targetName + ";" +
                         std::to_string(x) + ";" + std::to_string(y) + ";\n";
    oceanPortTemplate.append(assembledLine);
  }

  Fwg::Parsing::writeFile(path + "//ports.csv", oceanPortTemplate);

  auto defaultMapTemplate = Fwg::Parsing::readFile(Cfg::Values().resourcePath +
                                                   "eu5/map_data/default.map");
  Fwg::Parsing::replaceOccurences(defaultMapTemplate, "templateSeaZones",
                                  oceanZones);
  Fwg::Parsing::replaceOccurences(defaultMapTemplate, "templateLakes",
                                  lakeZones);
  Fwg::Parsing::writeFile(path + "//default.map", defaultMapTemplate);
  std::string definitionFileData = "";

  for (auto &continent : continents) {
    std::string continentData = std::to_string(continent->ID) + " = {\n";
    continentData.append("\t" + std::to_string(continent->ID) + "_sub = {\n");
    continentData.append("\t\t" + std::to_string(continent->ID) + "_reg = {\n");
    continentData.append("\t\t\t" + std::to_string(continent->ID) +
                         "_area = {\n");
    for (auto &region : continent->ardaRegions) {
      continentData.append("\t\t\t\t" + std::to_string(region->ID) +
                           "_province = {\n");
      for (auto &province : region->ardaProvinces) {
        continentData.append("\t\t\t\t\tprov_" + std::to_string(province->ID) +
                             "\n");
      }
      continentData.append("\t\t\t\t}\n");
    }
    continentData.append("\t\t\t}\n");
    continentData.append("\t\t}\n");
    continentData.append("\t}\n");
    continentData.append("}\n");
    definitionFileData.append(continentData);
  }

  std::string continentData = "water_1 = {\n";
  continentData.append("\twater_1_sub = {\n");
  continentData.append("\t\twater_1_reg = {\n");
  continentData.append("\t\twater_1_area = {\n");
  for (auto &region : regions) {
    if (!region->isSea()) {
      continue;
    }
    continentData.append("\t\t\t\t" + std::to_string(region->ID) +
                         "_province = {\n");
    for (auto &province : region->ardaProvinces) {
      continentData.append("\t\t\t\t\tprov_" + std::to_string(province->ID) +
                           "\n");
    }
    continentData.append("\t\t\t\t}\n");
  }
  continentData.append("\t\t\t}\n");
  continentData.append("\t\t}\n");
  continentData.append("\t}\n");
  continentData.append("}\n");
  definitionFileData.append(continentData);

  Fwg::Parsing::writeFile(path + "//definitions.txt", definitionFileData);

  Fwg::Parsing::writeFile(
      path + "//adjacencies.csv",
      "From;To;Type;Through;start_x;start_y;stop_x;stop_y;Comment");
}

} // namespace Rpx::Gfx::Eu5