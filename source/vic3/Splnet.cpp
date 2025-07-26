#include "vic3/Splnet.h"

namespace Scenario::Vic3 {
// method to create a float from 4 bytes
float createFloat(const char *bytes) {
  float f;
  memcpy(&f, bytes, sizeof(f));
  return f;
}

// method to create int from 4 bytes
int createInt(const char *bytes) {
  int i;
  // print all 4 bytes as hex
  // for (int i = 0; i < 4; i++) {

  //  std::cout << std::hex << (int)bytes[i] << " ";
  //}
  // std::cout << std::endl;
  memcpy(&i, bytes, sizeof(i));
  // std::cout << i << std::endl;
  return i;
}

// Function to find the pattern in the buffer starting from a specific index
// Returns the index to the start of the pattern, or -1 if not found
int findPatternFrom(const std::vector<char> &buffer,
                    const std::vector<char> &pattern, size_t startIndex = 0) {
  for (size_t i = startIndex; i <= buffer.size() - pattern.size(); ++i) {
    bool found = true;
    for (size_t j = 0; j < pattern.size(); ++j) {
      if (buffer[i + j] != pattern[j]) {
        found = false;
        break;
      }
    }
    if (found)
      return i;
  }
  return -1;
}

// Function to split a vector into chunks based on a pattern
std::vector<std::vector<char>>
splitVectorByPattern(const std::vector<char> &buffer,
                     const std::vector<char> &pattern) {
  std::vector<std::vector<char>> chunks;
  size_t startIndex = 0;
  int patternIndex = 0;

  while ((patternIndex = findPatternFrom(buffer, pattern, startIndex)) != -1) {
    std::vector<char> chunk(buffer.begin() + startIndex - pattern.size(),
                            buffer.begin() + patternIndex);
    chunks.push_back(chunk);
    std::cout << "Chunk size: " << chunk.size() << std::endl;
    startIndex = patternIndex + pattern.size();
  }

  // Add the last chunk if there's any data left after the last pattern
  if (startIndex < buffer.size()) {
    std::vector<char> lastChunk(buffer.begin() + startIndex - pattern.size(),
                                buffer.end());
    chunks.push_back(lastChunk);
  }

  return chunks;
}

// Function to find the pattern in the buffer
// Returns the index to the start of the pattern, or -1 if not found
int findPattern(const std::vector<char> &buffer,
                const std::vector<char> &pattern) {
  for (size_t i = 0; i <= buffer.size() - pattern.size(); ++i) {
    bool found = true;
    for (size_t j = 0; j < pattern.size(); ++j) {
      if (buffer[i + j] != pattern[j]) {
        found = false;
        break;
      }
    }
    if (found)
      return i;
  }
  return -1;
}

void splitStreamByPattern(std::istream &stream,
                          const std::vector<char> &splitPattern,
                          std::vector<char> &beforePattern,
                          std::vector<char> &afterPattern) {
  std::vector<char> buffer(std::istreambuf_iterator<char>(stream), {});

  int patternIndex = findPattern(buffer, splitPattern);
  if (patternIndex != -1) {
    // Copy everything before the pattern to beforePattern
    beforePattern.assign(buffer.begin(), buffer.begin() + patternIndex);
    // Copy everything after the pattern to afterPattern
    afterPattern.assign(buffer.begin() + patternIndex + splitPattern.size(),
                        buffer.end());
  } else {
    // If pattern not found, copy the whole buffer to beforePattern
    beforePattern = buffer;
  }
}

// for every region, create 5 anchors if possible, with the positions of the
// ardaProvinces
void Splnet::constructSplnet(
    const std::vector<std::shared_ptr<Arda::ArdaRegion>> &regions) {
  using namespace Fwg::Civilization;
  std::map<LocationType, int> locatorTypeToID = {
      {LocationType::City, 0},      {LocationType::Farm, 1},
      {LocationType::Mine, 2},      {LocationType::Port, 3},
      {LocationType::WaterPort, 0}, {LocationType::Forest, 4}};

  // a map from the location to an anchor ID
  std::map<std::shared_ptr<Location>, unsigned int> locationToAnchorID;
  int subAnchorCounter = 0;
  int stripIdCounter = 0;
  auto width = Fwg::Cfg::Values().width;
  for (auto &region : regions) {
    for (auto &location : region->significantLocations) {
      Anchor anchor;
      if (location->type == LocationType::WaterNode) {
        anchor.ID = (1 + region->ID) * 1000;
        // now shift bit 23 to 1
        anchor.ID |= 0x800000;
      } else if (location->type == LocationType::WaterPort) {
        anchor.ID = (1 + region->ID) * 100 + locatorTypeToID.at(location->type);
        anchor.ID |= 0x800000;
      } else {
        anchor.ID = (1 + region->ID) * 100 + locatorTypeToID.at(location->type);
      }
      anchor.xPos = location->position.widthCenter;
      anchor.yPos = location->position.heightCenter;
      locationToAnchorID[location] = anchor.ID;
      anchors.push_back(anchor);
    }
  }
  // now create the strips
  for (auto &region : regions) {
    // pairs of src and destination of already used connections
    std::set<std::pair<std::shared_ptr<Location>, std::shared_ptr<Location>>>
        usedConnections;
    for (auto &source : region->significantLocations) {
      // create a strip for every connection of this location, if not already
      // in usedConnections
      for (auto &connection : source->connections) {
        const auto &destination = connection.second.destination;
        // if the destination we're currently trying to reach, has not
        // created a connection to us yet
        if (usedConnections.find({destination, source}) ==
            usedConnections.end()) {
          Strip strip;

          strip.ID2 = 0;
          if (source == nullptr) {
            std::cout << "NULL" << std::endl;
          }

          try {
            if (locationToAnchorID.find(source) == locationToAnchorID.end()) {

              std::cerr << "Source not found in locationToAnchorID"
                        << std::endl;
              continue;
            }
          } catch (const std::exception &e) {
            continue;
          }
          auto startID = 0;
          auto targetID = 0;
          try {
            startID = locationToAnchorID.at(source);
            targetID = locationToAnchorID.at(destination);
            strip.anchorEntries.push_back(
                StripAnchorEntry{0x14, locationToAnchorID.at(source)});
          } catch (const std::exception &e) {
            continue;
          }
          // if the startAnchor is higher than the targetAnchor, swap them
          if (startID > targetID || startID == targetID ||
              source->land != destination->land) {
            // int temp = strip.startAnchor;
            // strip.startAnchor = strip.targetAnchor;
            // strip.targetAnchor = temp;
            continue;
          }
          strip.ID = stripIdCounter++ * 256;
          // now add all connecting pixels as subanchors to the anchors list
          // and as a subanchor to the strip
          for (auto &pixel : connection.second.connectingPixels) {
            Anchor anchor;
            anchor.ID = subAnchorCounter++;

            // now shift bit 28 to 1
            anchor.ID |= 0x10000000;
            anchor.xPos = pixel % width;
            anchor.yPos = pixel / width;
            anchors.push_back(anchor);
            strip.anchorEntries.push_back(StripAnchorEntry{0x14, anchor.ID});
          }

          strip.anchorEntries.push_back(
              StripAnchorEntry{0x14, locationToAnchorID.at(destination)});
          strips.push_back(strip);
          // push a new connection from us to the destination
          usedConnections.insert({source, destination});

          Segment segment;
          int typeAdditive = 0;
          if (source->type == LocationType::WaterNode &&
              destination->type == LocationType::WaterNode) {
            typeAdditive = 2;
          } else if ((source->type == LocationType::WaterPort &&
                      destination->type == LocationType::WaterNode) ||
                     (source->type == LocationType::WaterNode &&
                      destination->type == LocationType::WaterPort)) {
            typeAdditive = 3;
          }
          segment.Idblock0 = startID * 64 + typeAdditive;
          segment.IDmult800 = targetID * 8;
          segment.refStripId = strip.ID;
          segment.refStripId2 = strip.ID2;
          segments.push_back(segment);
        }
      }
    }
  }

  // sort the anchors, strips and segments
  std::sort(anchors.begin(), anchors.end(),
            [](const Anchor &a, const Anchor &b) { return a.ID < b.ID; });
  std::sort(strips.begin(), strips.end(),
            [](const Strip &a, const Strip &b) { return a.ID < b.ID; });
  std::sort(segments.begin(), segments.end(),
            [](const Segment &a, const Segment &b) {
              return a.Idblock0 < b.Idblock0;
            });

  if (strips.size())
    strips.back().unknown11 = 0x04;
  if (segments.size())
    segments.back().unknown13 = 0x04;
  if (anchors.size())
    anchors.back().unknown9 = 4;
  header.anchorAmount = anchors.size();
  header.stripAmount = strips.size();
  header.segmentAmount = segments.size();
  if (segments.size() != strips.size()) {
    std::cerr << "Segments and strips size mismatch" << std::endl;
  }
}
void Splnet::parseHeader(const std::array<char, 36> &headerData,
                         Header &header) {
  header.anchorAmount = createInt(headerData.data() + 18);
  header.stripAmount = createInt(headerData.data() + 24);
  std::cout << "Spline amount 1: " << header.stripAmount << std::endl;
  header.segmentAmount = createInt(headerData.data() + 30);
  std::cout << "Spline amount 2: " << header.segmentAmount << std::endl;
}
void Splnet::parseAnchor(const std::array<char, 34> &anchorData,
                         Anchor &anchor) {
  // Assuming the buffer is in little-endian format and needs to be read as
  // such.
  int offset = 0;

  // Read unsigned short values
  anchor.unknown1 =
      *reinterpret_cast<const unsigned short *>(anchorData.data() + offset);
  offset += sizeof(unsigned short);

  anchor.unknown2 =
      *reinterpret_cast<const unsigned short *>(anchorData.data() + offset);
  offset += sizeof(unsigned short);

  anchor.unknown3 =
      *reinterpret_cast<const unsigned short *>(anchorData.data() + offset);
  offset += sizeof(unsigned short);

  // Read unsigned int value
  anchor.ID =
      *reinterpret_cast<const unsigned int *>(anchorData.data() + offset);
  offset += sizeof(unsigned int);

  // Continue reading unsigned short values
  anchor.type2 =
      *reinterpret_cast<const unsigned short *>(anchorData.data() + offset);
  offset += sizeof(unsigned short);

  anchor.type3 =
      *reinterpret_cast<const unsigned short *>(anchorData.data() + offset);
  offset += sizeof(unsigned short);

  anchor.unknown4 =
      *reinterpret_cast<const unsigned short *>(anchorData.data() + offset);
  offset += sizeof(unsigned short);

  anchor.unknown5 =
      *reinterpret_cast<const unsigned short *>(anchorData.data() + offset);
  offset += sizeof(unsigned short);

  anchor.xPos = createFloat(anchorData.data() + offset);
  offset += sizeof(float);

  anchor.unknown6 =
      *reinterpret_cast<const unsigned short *>(anchorData.data() + offset);
  offset += sizeof(unsigned short);

  anchor.yPos = createFloat(anchorData.data() + offset);
  offset += sizeof(float);

  // Continue reading the rest of the unsigned short values
  anchor.unknown7 =
      *reinterpret_cast<const unsigned short *>(anchorData.data() + offset);
  offset += sizeof(unsigned short);

  anchor.unknown8 =
      *reinterpret_cast<const unsigned short *>(anchorData.data() + offset);
  offset += sizeof(unsigned short);

  anchor.unknown9 =
      *reinterpret_cast<const unsigned short *>(anchorData.data() + offset);
  offset += sizeof(unsigned short);
}

Segment vectorToStrip2(const std::vector<char> &data) {
  Segment strip2;
  if (data.size() < sizeof(Segment)) {
    std::cerr << "Data vector is too small to convert to Strip2" << std::endl;
    return strip2; // Return default-initialized strip2 or handle error
                   // appropriately
  }

  // Directly copy the data into strip2
  std::memcpy(&strip2, data.data(), sizeof(Segment));

  return strip2;
}

Strip vectorToStrip(const std::vector<char> &data) {
  Strip strip;
  if (data.size() < sizeof(Strip)) {
    std::cerr << "Data vector is too small to convert to Strip" << std::endl;
    return strip; // Return default-initialized strip or handle error
                  // appropriately
  }

  size_t offset = 0;

  // Assuming the data vector is properly aligned and packed as per Strip struct
  strip.unknown1 = *reinterpret_cast<const unsigned short *>(&data[offset]);
  offset += sizeof(unsigned short);

  strip.unknown2 = *reinterpret_cast<const unsigned short *>(&data[offset]);
  offset += sizeof(unsigned short);

  strip.unknown3 = *reinterpret_cast<const unsigned short *>(&data[offset]);
  offset += sizeof(unsigned short);

  strip.ID = *reinterpret_cast<const unsigned int *>(&data[offset]);
  offset += sizeof(unsigned int);
  strip.ID2 = *reinterpret_cast<const unsigned int *>(&data[offset]);
  offset += sizeof(unsigned int);
  strip.unknown4 = *reinterpret_cast<const unsigned short *>(&data[offset]);
  offset += sizeof(unsigned short);

  strip.unknown5 = *reinterpret_cast<const unsigned short *>(&data[offset]);
  offset += sizeof(unsigned short);

  strip.unknown6 = *reinterpret_cast<const unsigned short *>(&data[offset]);
  offset += sizeof(unsigned short);

  // strip.unknown7 = *reinterpret_cast<const unsigned short *>(&data[offset]);
  // offset += sizeof(unsigned short);

  // strip.startAnchor = createInt(data.data() + offset);
  // offset += sizeof(unsigned int);

  // strip.unknown8 = *reinterpret_cast<const unsigned short *>(&data[offset]);
  // offset += sizeof(unsigned short);

  // strip.targetAnchor = createInt(data.data() + offset);
  // offset += sizeof(unsigned int);

  strip.unknown9 = *reinterpret_cast<const unsigned short *>(&data[offset]);
  offset += sizeof(unsigned short);

  strip.unknown10 = *reinterpret_cast<const unsigned short *>(&data[offset]);
  offset += sizeof(unsigned short);

  strip.unknown11 = *reinterpret_cast<const unsigned short *>(&data[offset]);
  offset += sizeof(unsigned short);

  return strip;
}

void Splnet::parseFile(const std::string &path) {
  std::ifstream stream(path.c_str(), std::ios::binary);

  if (!stream) {
    std::cerr << "Failed to open file: " << path << std::endl;
    return;
  }
  std::array<char, 36> headerData;
  read_from_stream(stream, headerData);
  parseHeader(headerData, header);
  header.printHeader();

  read_from_stream(stream, anchorHeader);
  for (int i = 0; i < header.anchorAmount; i++) {

    Anchor anchor;
    std::array<char, 34> anchorData;
    read_from_stream(stream, anchorData);
    parseAnchor(anchorData, anchor);
    anchors.push_back(anchor);
  }
  // now, read strip header
  read_from_stream(stream, stripHeader);

  std::vector<char> splitPattern = {'\xf6', '\x05', '\x01', '\x00',
                                    '\x03', '\x00', '\x03', '\x00'};
  std::vector<char> stripData1, stripData2;

  splitStreamByPattern(stream, splitPattern, stripData1, stripData2);

  // Example usage: Print sizes of the vectors
  std::cout << "Before pattern size: " << stripData1.size() << std::endl;
  std::cout << "After pattern size: " << stripData2.size() << std::endl;

  //  now read from next byte to next 0x0b, 0x00, 0x01, 0x00
  //  Define the pattern you're looking for
  std::vector<char> pattern = {'\x0b', '\x00', '\x01', '\x00'};
  auto strip1Chunks = splitVectorByPattern(stripData1, pattern);
  auto strip2Chunks = splitVectorByPattern(stripData2, pattern);
  //  now read the strip data
  for (auto &chunk : strip1Chunks) {
    if (chunk.size() < sizeof(Strip)) {
      std::cerr << "Chunk is too small to be a Strip" << std::endl;
      continue;
    }

    Strip strip = vectorToStrip(chunk);
    strips.push_back(strip);
  }
  std::set<int> stripIds;
  //  now read the strip data
  for (auto &chunk : strip2Chunks) {
    if (chunk.size() < sizeof(Segment)) {
      std::cerr << "Chunk is too small to be a Strip2" << std::endl;
      continue;
    }

    Segment strip2 = vectorToStrip2(chunk);
    // stripIds.insert(strip2.ID);
    segments.push_back(strip2);
  }
  for (auto &strip : stripIds) {
    std::cout << "Strip ID: " << strip << std::endl;
  }
}

void Splnet::writeFile(const std::string &path) {
  std::ofstream stream(path.c_str(), std::ios::binary);

  if (!stream) {
    std::cerr << "Failed to open file: " << path << std::endl;
    return;
  }

  stream.write(reinterpret_cast<const char *>(&header), sizeof(header));
  stream.write(reinterpret_cast<const char *>(&anchorHeader),
               sizeof(anchorHeader));
  for (int i = 0; i < header.anchorAmount; i++) {
    stream.write(reinterpret_cast<const char *>(&anchors[i]), sizeof(Anchor));
  }
  if (strips.size()) {
    stream.write(reinterpret_cast<const char *>(&stripHeader),
                 sizeof(stripHeader));
    for (auto &strip : strips) {
      // Write the Strip structure up to the anchorEntries vector
      stream.write(reinterpret_cast<const char *>(&strip.unknown1),
                   sizeof(strip.unknown1));
      stream.write(reinterpret_cast<const char *>(&strip.unknown2),
                   sizeof(strip.unknown2));
      stream.write(reinterpret_cast<const char *>(&strip.unknown3),
                   sizeof(strip.unknown3));
      stream.write(reinterpret_cast<const char *>(&strip.ID), sizeof(strip.ID));
      stream.write(reinterpret_cast<const char *>(&strip.ID2),
                   sizeof(strip.ID2));
      stream.write(reinterpret_cast<const char *>(&strip.unknown4),
                   sizeof(strip.unknown4));
      stream.write(reinterpret_cast<const char *>(&strip.unknown5),
                   sizeof(strip.unknown5));
      stream.write(reinterpret_cast<const char *>(&strip.unknown6),
                   sizeof(strip.unknown6));

      // Write each StripAnchorEntry in the vector
      for (const auto &entry : strip.anchorEntries) {
        stream.write(reinterpret_cast<const char *>(&entry),
                     sizeof(StripAnchorEntry));
      }

      // Write the remaining fields of the Strip structure
      stream.write(reinterpret_cast<const char *>(&strip.unknown9),
                   sizeof(strip.unknown9));
      stream.write(reinterpret_cast<const char *>(&strip.unknown10),
                   sizeof(strip.unknown10));
      stream.write(reinterpret_cast<const char *>(&strip.unknown11),
                   sizeof(strip.unknown11));
    }

    stripEndHeader = stripHeader;
    stripEndHeader.unknown1 = 0x05f6;

    stream.write(reinterpret_cast<const char *>(&stripEndHeader),
                 sizeof(stripHeader));
    for (auto &strip2 : segments) {
      stream.write(reinterpret_cast<const char *>(&strip2), sizeof(Segment));
    }
  }

  stream.close();
}
} // namespace Scenario::Vic3