#pragma once

#include <iostream>
#include <random>
#include <windows.h>

#include <boost/geometry/geometries/linestring.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/multi_point.hpp>
#include <boost/geometry/geometries/multi_polygon.hpp>
#include <boost/geometry.hpp>

#include "../FastWorldGen/FastWorldGen/FastWorldGenerator.h"

namespace bg = boost::geometry;
using namespace std;
class Flag
{
	ranlux24 random;
	vector<Colour> colours;
	vector<uint8_t> flag;
	enum type { TRICOLORE, ROTATEDTRICOLORE, PLAIN, };
	enum symbolType { CIRCLE, SQUARE, MOON, STAR, MOONSTAR, MULTISTAR };
	type flagType;
	symbolType symbolType;

public:
	int width;
	int height;
	Flag();
	Flag(ranlux24 random, int width, int height);
	~Flag();
	void tricolore(int i, int j);
	void rotatedTricolore(int i, int j);
	void plain(int i, int j);
	void squareSquared(int i, int j);
	void circle(int i, int j);
	void halfMoon(int i, int j );
	void star(int i, int j, double xPos, double yPos, double size);
	void halfMoonStars(int i, int j);
	//void triangle(int i, int j, )
	vector<Colour> generateColours();
	void setPixel(Colour colour, uint32_t x, uint32_t y);
	Colour getPixel(uint32_t x, uint32_t y);
	Colour getPixel(uint32_t pos);
	vector<uint8_t> getFlag();
	vector<uint8_t> resize(int width, int hight);
};
