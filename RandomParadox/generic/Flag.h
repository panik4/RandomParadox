#pragma once
#include <boost/geometry/geometries/linestring.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/multi_point.hpp>
#include <boost/geometry/geometries/multi_polygon.hpp>
#include <boost/geometry.hpp>
#include "../FastWorldGen/FastWorldGen/FastWorldGenerator.h"
#include "ParserUtils.h"

namespace bg = boost::geometry;
class Flag
{
	// static cause we only want to read them from file once
	static std::map<std::string, std::vector<Colour>> colourGroups;
	static std::vector<std::vector<std::vector<int>>> flagTypes;
	static std::vector<std::vector<std::vector<std::string>>> flagTypeColours;
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
	// methods - shapes
	void tricolore(int i, int j);
	void rotatedTricolore(int i, int j);
	void bicolore(int i, int j);
	void rotatedBicolore(int i, int j);
	void plain(int i, int j);
	void squareSquared(int i, int j);
	void circle(int i, int j);
	void halfMoon(int i, int j);
	void star(int i, int j, double xPos, double yPos, double size);
	void halfMoonStars(int i, int j);
	void triangle(int i, int j, double xPos, double yPos, double size);
	// methods - image read/write
	void setPixel(Colour colour, int x, int y);
	Colour getPixel(int x, int y);
	Colour getPixel(int pos);
	std::vector<unsigned char> getFlag();
	// methods - utils
	std::vector<unsigned char> resize(int width, int hight);
	// methods - read in configs
	static void readColourGroups();
	static void readFlagTypes();
};

