#pragma once
#include "../FastWorldGen/FastWorldGen/FastWorldGenerator.h"
#include "TextureWriter.h"
#include "ParserUtils.h"

class Flag
{
	// static cause we only want to read them from file once
	static std::map<std::string, std::vector<Colour>> colourGroups;
	static std::vector<std::vector<std::vector<int>>> flagTypes;
	static std::vector<std::vector<std::vector<std::string>>> flagTypeColours;
	static std::vector<std::vector<uint8_t>> flagTemplates;
	static std::vector<std::vector<std::string>> flagMetadata;
	static std::vector<std::vector<uint8_t>> symbolTemplates;
	static std::vector<std::vector<std::string>> symbolMetadata;
	std::ranlux24 random;
	// containers
	std::vector<Colour> colours;
	std::vector<unsigned char> image;
	enum type { TRICOLORE, ROTATEDTRICOLORE, PLAIN, PLAIN_TRIANGLE, BICOLORE, BICOLORE_TRIANGLE, ROTATEDBICOLORE };
	enum symbolType { CIRCLE, SQUARE, MOON, STAR, MOONSTAR, MULTISTAR, LEFT_TRIANGLE };
	type flagType;
	symbolType symbolType;

public:
	// vars
	int width;
	int height;
	// constructors/destructors
	Flag();
	Flag(std::ranlux24 random, int width, int height);
	~Flag();
	// methods - image read/write
	void setPixel(Colour colour, int x, int y);
	Colour getPixel(int x, int y);
	Colour getPixel(int pos);
	std::vector<unsigned char> getFlag() const;
	// methods - utils
	std::vector<unsigned char> resize(int width, int height) const;
	static std::vector<unsigned char> resize(int width, int height, std::vector<unsigned char> tImage, int inWidth, int inHeight);
	// methods - read in configs
	static void readColourGroups();
	static void readFlagTypes();
	static void readFlagTemplates();
	static void readSymbolTemplates();
};

