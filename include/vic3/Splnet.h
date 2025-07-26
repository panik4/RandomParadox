#pragma once
#include "areas/ArdaRegion.h"
#include "utils/ParsingUtils.h"
#include <vector>
namespace Scenario::Vic3 {

#pragma pack(push, 1) // Disable padding bytes

struct Header {
  unsigned short unknown1 = 0xee;
  unsigned short unknown2 = 0x01;
  unsigned short unknown3 = 0x0c;
  unsigned short unknown4 = 0x04;
  unsigned short unknown5 = 0x00;
  unsigned short unknown6 = 0x045A;
  unsigned short unknown7 = 0x01;
  unsigned short unknown8 = 0x03;
  unsigned short unknown9 = 0x0c;
  unsigned int anchorAmount = 0x00;
  unsigned short unknown10 = 0x0c;
  unsigned int stripAmount = 0x00;
  unsigned short unknown11 = 0x0c;
  unsigned int segmentAmount = 0x00;
  unsigned short unknown12 = 0x04;

  // print everything in one line with spaces in between
  void printHeader() {
	std::cout << unknown1 << " " << unknown2 << " " << unknown3 << " " << unknown4
			  << " " << unknown5 << " " << unknown6 << " " << unknown7 << " "
			  << unknown8 << " " << unknown9 << " " << anchorAmount << " "
			  << unknown10 << " Strips: " << stripAmount << " " << unknown11 << " Segment: "
			  << segmentAmount << " " << unknown12 << std::endl;
  }
};
struct AnchorHeader {
  // 8 byte magic after adding first anchor
  unsigned short unknown1 = 0x05f4;
  unsigned short unknown2 = 0x01;
  unsigned short unknown3 = 0x03;
  unsigned short unknown4 = 0x03;
};

struct Anchor {
  unsigned short unknown1 = 11;
  unsigned short unknown2 = 1;
  unsigned short unknown3 = 20;
  unsigned int ID;
  unsigned short type2 = 76;
  unsigned short type3 = 1;
  unsigned short unknown4 = 3;
  unsigned short unknown5 = 13;
  float xPos;
  unsigned short unknown6 = 13;
  float yPos;
  unsigned short unknown7 = 4;
  unsigned short unknown8 = 4;
  unsigned short unknown9 = 3;

  // print everything in one line with spaces in between
  void printAnchor() {
    std::cout << unknown1 << " " << unknown2 << " " << unknown3 << " " << ID
              << " " << type2 << " " << type3 << " " << unknown4 << " "
              << unknown5 << " " << xPos << " " << unknown6 << " " << yPos
              << " " << unknown7 << " " << unknown8 << " " << unknown9
              << std::endl;
  }
};

struct StripHeader {
  // 8 byte magic after adding first anchor
  unsigned short unknown1 = 0x05f5;
  unsigned short unknown2 = 0x01;
  unsigned short unknown3 = 0x03;
  unsigned short unknown4 = 0x03;
};

struct StripAnchorEntry {
  unsigned short prefix = 0x14;
  unsigned int anchorID = 0;
};

// 38 byte minimum size, + subanchor IDs
struct Strip {
  unsigned short unknown1 = 0x0b;
  unsigned short unknown2 = 0x01;
  unsigned short unknown3 = 0x029c;
  unsigned int ID = 0;
  unsigned int ID2 = 0;
  unsigned short unknown4 = 0x05f7;
  unsigned short unknown5 = 0x01;
  unsigned short unknown6 = 0x03;
  std::vector<StripAnchorEntry> anchorEntries;
  unsigned short unknown9 = 0x04;
  unsigned short unknown10 = 0x04;
  unsigned short unknown11 = 0x03;
};



struct Segment {
  unsigned short unknown1 = 0x0b;
  unsigned short unknown2 = 0x01;
  unsigned short unknown3 = 0x029c;
  // city with anything 0x00
  // wood - farm: 0x40
  // wood - mine: 0x80
  // farm - mine: 0x40
  // 

  //unsigned char connectionTypeDef = 0;
  //unsigned short IDmult25 = 0x00;
  //unsigned char buffer1 = 0x00;
  // contains connection type, as well as the ID of the connection * 6400
  unsigned int Idblock0;

  // some type of multiplier? seems to be ID * 800 +  some value
  // some value depends on connection type: woodd-mine + 32, 
  unsigned int IDmult800 = 0;
  unsigned short unknown7 = 0x05f5;
  unsigned short unknown8 = 0x01;
  unsigned short unknown9 = 0x03;
  unsigned short unknown10 = 0x029c;
  unsigned int refStripId = 0x00;
  unsigned int refStripId2 = 0x00;
  unsigned short unknown11 = 0x04;
  unsigned short unknown12 = 0x04;
  unsigned short unknown13 = 0x03;

  void printStrip2() {

  }
};
#pragma pack(pop) // Restore the default packing
template <typename T>
inline void read_from_stream(std::ifstream &stream, T &t) {
  stream.read(reinterpret_cast<char *>(&t), sizeof(T));
}
class Splnet {
  Header header;
  AnchorHeader anchorHeader;
  StripHeader stripHeader;
  StripHeader stripEndHeader;

  std::vector<Anchor> anchors;
  std::vector<Strip> strips;
  std::vector<Segment> segments;

public:
  void constructSplnet(
      const std::vector<std::shared_ptr<Arda::ArdaRegion>> &regions);

  // parse header
  void parseHeader(const std::array<char, 36> &headerData, Header &header);
  void parseAnchor(const std::array<char, 34> &anchorData, Anchor &anchor);
  // parse file
  void parseFile(const std::string &path);

  // write file
  void writeFile(const std::string &path);
};

} // namespace Scenario::Vic3