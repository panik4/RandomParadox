#include "Flag.h"
map<std::string, vector<Colour>> Flag::colourGroups;
vector<vector<vector<int>>> Flag::flagTypes(7);
vector<vector<vector<std::string>>> Flag::flagTypeColours(7);
Flag::Flag()
{
}

Flag::Flag(ranlux24 random, int width, int height) : random(random), width(width), height(height)
{

	// load the template
	//BYTE* targaimage;
	//targaimage = (BYTE*)tga_load("C:\\Users\\username\\Documents\\Visual Studio 2017\\Projects\\\RandomVic2\\RandomVic2\\resources\\gfx\\flags\\template.tga", &width, &height, TGA_TRUECOLOR_32);
	//this->flag = targaimage;
	flag = std::vector<uint8_t>(width * height * 4, 0);
	int type = random() % flagTypes.size();
	int flagSubType = random() % flagTypes[type].size();
	int symbolType = flagTypes[type][flagSubType].size() ? *select_random(flagTypes[type][flagSubType]) : 0;
	
	//colours = generateColours();
	auto randomIndex = random() % flagTypeColours[type].size();
	for (auto& colGroup : flagTypeColours[type][flagSubType])
	{
			
		auto colour = *select_random(colourGroups[colGroup]);
		if (colours.size())
			while (colour == colours[colours.size() - 1])
			{
				colour = *select_random(colourGroups[colGroup]);
			}
		// symbol must not have the same colour as any of the previous flag colours
		if (colGroup == flagTypeColours[type][flagSubType].back())
		{
		
		}

		colours.push_back(colour);
	}
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			switch (type)
			{
			case 0:
			{
				flagType = TRICOLORE;
				tricolore(i, j);
				break;
			}
			case 1:
			{
				flagType = ROTATEDTRICOLORE;
				rotatedTricolore(i, j);
				break;
			}
			case 2:
			{
				flagType = PLAIN;
				plain(i, j);
				break;
			}
			case 3:
			{
				flagType = PLAIN_TRIANGLE;
				plain(i, j);
				triangle(i, j, -0.1, 0.5, 0.55);
				break;
			}
			case 4:
			{
				flagType = BICOLORE;
				bicolore(i, j);
				break;
			}
			case 5:
			{
				flagType = ROTATEDBICOLORE;
				rotatedBicolore(i, j);
				break;
			}
			case 6:
			{
				flagType = BICOLORE_TRIANGLE;
				rotatedBicolore(i, j);
				triangle(i, j, -0.1, 0.5, 0.55);
				break;
			}
			default:
				flagType = PLAIN;
				plain(i, j);
				break;
			}
		}
	}
	//colours = generateColours();
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			switch (symbolType)
			{
			case 0:
			{
				break;
			}
			case 20:
			{
				flagSubType = STAR;
				star(i, j, 0.5, 0.5, 0.35);
				break;
			}
			case 21:
			{
				flagSubType = MOONSTAR;
				halfMoonStars(i, j);
				break;
			}
			case 22:
			{
				flagSubType = SQUARE;
				squareSquared(i, j);
				break;
			}
			case 23:
			{
				flagSubType = CIRCLE;
				circle(i, j);
				break;
			}
			case 24:
			{
				flagSubType = MULTISTAR;
				circle(i, j);
				break;
			}
			case 25:
			{
				flagSubType = MOON;
				halfMoon(i, j);
				break;
			}
			case 26:
			{
				flagSubType = LEFT_TRIANGLE;
				triangle(i, j, -0.1, 0.5, 0.45); 
				break;
			}
			default:
				break;
			}
		}
	}
}

Flag::~Flag()
{
}

void Flag::tricolore(int i, int j)
{
	unsigned short colourIndex = j / (width / 3);
	setPixel(colours[colourIndex], i, j);
}

void Flag::rotatedTricolore(int i, int j)
{
	unsigned short colourIndex = i / (height / 3);
	setPixel(colours[colourIndex], i, j);
}

void Flag::bicolore(int i, int j)
{
	unsigned short colourIndex = j / (width / 2);
	setPixel(colours[colourIndex], i, j);
}

void Flag::rotatedBicolore(int i, int j)
{
	unsigned short colourIndex = i / (height / 2);
	setPixel(colours[colourIndex], i, j);
}

void Flag::plain(int i, int j)
{
	setPixel(colours[0], i, j);
}

void Flag::squareSquared(int i, int j)
{
	if (abs((int)j - width / 2) < 16 && abs((int)i - height / 2) < 16)
		setPixel(colours.back(), i, j);
}

void Flag::circle(int i, int j)
{
	struct Point { int x; int y; };
	Point center{ width / 2, height / 2 };
	Point curPos{ j, i };
	auto distance = abs(std::hypot(center.x - curPos.x, center.y - curPos.y));
	if (distance < 15) {
		setPixel(colours.back(), i, j);
	}
}

void Flag::halfMoon(int i, int j)
{
	auto radius = 15.0;
	struct Point { int x; int y; };
	Point center{ width / 2, height / 2 };
	Point curPos{ j, i };
	if (curPos.x < center.x + radius)
	{
		double distanceFromLeftMost = fabs(curPos.x - (center.x - radius));
		double factor = (distanceFromLeftMost / (radius));
		auto distance = std::hypot(center.x - curPos.x, center.y - curPos.y);
		if (distance < radius && distance > radius * 0.5 * (factor)) {
			setPixel(colours.back(), i, j);
		}
	}
}
bg::model::point<double, 2, bg::cs::cartesian> rotate(double angle, bg::model::point<double, 2, bg::cs::cartesian> point, bg::model::point<double, 2, bg::cs::cartesian> pivot)
{
	//double angle = 1.2566370614;
	double s = sin(angle); // angle is in radians
	double c = cos(angle); // angle is in radians

	double xnew = (bg::get<0>(point) - bg::get<0>(pivot)) * c + (bg::get<1>(point) - bg::get<1>(pivot)) * s;
	double ynew = -(bg::get<0>(point) - bg::get<0>(pivot)) * s + (bg::get<1>(point) - bg::get<1>(pivot)) * c;
	bg::model::point<double, 2, bg::cs::cartesian> point2(xnew + bg::get<0>(pivot), ynew + bg::get<1>(pivot));
	return point2;

}
void Flag::star(int i, int j, double xPos, double yPos, double size)
{
	double angle = 72 * 3.14 / 180;
	typedef boost::geometry::model::d2::point_xy<double> point_type;
	typedef boost::geometry::model::linestring<point_type> linestring_type;


	bg::model::point<double, 2, bg::cs::cartesian> center(width * xPos, height * yPos);
	bg::model::point<double, 2, bg::cs::cartesian> curPos(j, i);
	vector<bg::model::point<double, 2, bg::cs::cartesian>> points;
	bg::model::point<double, 2, bg::cs::cartesian> one(bg::get<0>(center), bg::get<1>(center) + size * width); // up
	points.push_back(one);
	for (int i = 0; i < 4; i++)
	{
		points.push_back(rotate(angle, points[i], center));
	}
	vector<linestring_type> lines;
	for (auto point : points)
	{
		linestring_type line;
		line.push_back(point_type(bg::get<0>(center), bg::get<1>(center)));
		line.push_back(point_type(bg::get<0>(point), bg::get<1>(point)));
		lines.push_back(line);

	}
	for (int index = 0; index < points.size(); index++)
	{
		double lineDistance = fabs(bg::distance(curPos, lines[index]));
		double yDistance = bg::distance(curPos, points[index]);

		double centerDistance = bg::distance(curPos, center);
		if (yDistance < (size * width)) {

			double factor = yDistance / (size * (double)width);
			if (lineDistance < (width / 4 * size) * factor)
				setPixel(colours.back(), i, j);

		}
	}
}

void Flag::halfMoonStars(int i, int j)
{
	halfMoon(i, j);
	star(i, j, 0.65, 0.7, 0.07);
	star(i, j, 0.7, 0.5, 0.07);
	star(i, j, 0.65, 0.3, 0.07);
}

void Flag::triangle(int i, int j, double xPos, double yPos, double size)
{
	double angle = 90 * 3.14 / 180.0;
	typedef boost::geometry::model::d2::point_xy<double> point_type;
	typedef boost::geometry::model::linestring<point_type> linestring_type;


	bg::model::point<double, 2, bg::cs::cartesian> center(width * xPos, height * yPos);
	bg::model::point<double, 2, bg::cs::cartesian> curPos(j, i);
	vector<bg::model::point<double, 2, bg::cs::cartesian>> points;
	bg::model::point<double, 2, bg::cs::cartesian> one(bg::get<0>(center), bg::get<1>(center) + size * width); // up
	points.push_back(rotate(angle, one, center));
	for (int i = 0; i < 0; i++)
	{
		points.push_back(rotate(angle, points[i], center));
	}
	vector<linestring_type> lines;
	for (auto point : points)
	{
		linestring_type line;
		line.push_back(point_type(bg::get<0>(center), bg::get<1>(center)));
		line.push_back(point_type(bg::get<0>(point), bg::get<1>(point)));
		lines.push_back(line);
	}
	for (int index = 0; index < points.size(); index++)
	{
		double lineDistance = fabs(bg::distance(curPos, lines[index]));
		double yDistance = bg::distance(curPos, points[index]);

		double centerDistance = bg::distance(curPos, center);
		if (yDistance < (size * width)) {

			double factor = yDistance / (size * (double)width);
			if (lineDistance < (width / 2 * size) * factor)
				setPixel(colours.back(), i, j);

		}
	}
}

//TODO: Return pretty colour combinations
vector<Colour> Flag::generateColours()
{
	// NOTE: this is B, G , R!!!
	vector<Colour> neutrals;
	neutrals.push_back(Colour{ 0, 0, 0 }); // black
	neutrals.push_back(Colour{ 255, 255, 255 }); // white
	//neutrals.push_back(Colour{ 192, 192, 192 }); // silver

	vector<Colour> brightColours;
	brightColours.push_back(Colour{ 20, 20, 200 }); // red
	brightColours.push_back(Colour{ 34,139,34 }); // green
	brightColours.push_back(Colour{ 200, 30, 0 }); // blue
	brightColours.push_back(Colour{ 0, 215, 255 }); // gold

	vector<Colour> darkColours;
	darkColours.push_back(Colour{ 90, 30, 30 }); // dark blue
	darkColours.push_back(Colour{ 0, 100, 0 }); // dark green
	darkColours.push_back(Colour{ 20, 30, 100 }); // dark red
	darkColours.push_back(Colour{ 45, 82, 160 }); // gold-ish red

	vector<vector<Colour>> nonNeutralTypes;
	nonNeutralTypes.push_back(brightColours);
	nonNeutralTypes.push_back(darkColours);


	vector<Colour> colours;
	bool doubleNeutral = random() % 2;
	//if (doubleNeutral)
	//{
	//	colours.push_back(neutrals[random() % neutrals.size()]);
	//	vector<Colour> middleColour = nonNeutralTypes[random() % nonNeutralTypes.size()];
	//	colours.push_back(middleColour[random() % middleColour.size()]);
	//	colours.push_back(neutrals[random() % neutrals.size()]);
	//}
	{
		vector<Colour> leftColours = nonNeutralTypes[random() % nonNeutralTypes.size()];
		colours.push_back(leftColours[random() % leftColours.size()]);
		colours.push_back(neutrals[random() % neutrals.size()]);
		vector<Colour> rightColours = nonNeutralTypes[random() % nonNeutralTypes.size()];
		colours.push_back(rightColours[random() % rightColours.size()]);

		colours.push_back(brightColours[random() % brightColours.size()]);
		//while(colours[3] == colours)
	}
	return colours;
}

void Flag::setPixel(Colour colour, uint32_t x, uint32_t y)
{
	flag[(x * width + y) * 4] = colour.getRed();
	flag[(x * width + y) * 4 + 1] = colour.getGreen();
	flag[(x * width + y) * 4 + 2] = colour.getBlue();
	flag[(x * width + y) * 4 + 3] = 255;
}

Colour Flag::getPixel(uint32_t x, uint32_t y)
{
	Colour colour{ flag[(x * width + y) * 4],flag[(x * width + y) * 4 + 1],flag[(x * width + y) * 4 + 2] };
	return colour;
}


Colour Flag::getPixel(uint32_t pos)
{
	Colour colour{ flag[pos * 4] ,flag[pos * 4 + 1] ,flag[pos * 4 + 2] };
	return colour;
}

vector<uint8_t> Flag::getFlag()
{
	return flag;;
}

vector<uint8_t> Flag::resize(int width, int height)
{
	auto resized = std::vector<uint8_t>(width * height * 4, 0);
	auto factor = this->width / width;
	for (int h = 0; h < height; h++)
	{
		for (int w = 0; w < width; w++)
		{
			auto colourmapIndex = factor * h * this->width + factor * w;
			colourmapIndex *= 4;
			resized[(h * width + w) * 4] = flag[colourmapIndex];
			resized[(h * width + w) * 4 + 1] = flag[colourmapIndex + 1];
			resized[(h * width + w) * 4 + 2] = flag[colourmapIndex + 2];
			resized[(h * width + w) * 4 + 3] = flag[colourmapIndex + 3];
		}
	}
	return resized;
}

void Flag::readColourGroups()
{
	auto lines = ParserUtils::getLines("resources\\flags\\colour_groups.txt");
	for (auto& line : lines)
	{
		if (!line.size())
			continue;
		auto tokens = ParserUtils::getTokens(line, ';');
		for (int i = 1; i < tokens.size(); i++)
		{
			auto colour = ParserUtils::getNumbers(tokens[i], ',', std::set<int>{});
			colourGroups[tokens[0]].push_back(Colour(colour));
		}
	}
}

void Flag::readFlagTypes()
{
	auto lines = ParserUtils::getLines("resources\\flags\\flag_types.txt");
	for (auto& line : lines)
	{
		if (!line.size())
			continue;
		auto tokens = ParserUtils::getTokens(line, ';');
		const auto flagType = stoi(tokens[0]);
		const auto flagTypeID = flagTypes[flagType].size();
		auto symbols = ParserUtils::getTokens(tokens[1], ',');
		auto colourGroupStrings = ParserUtils::getTokens(tokens[2], ',');
		flagTypes[flagType].push_back(std::vector<int>{});
		flagTypeColours[flagType].push_back(std::vector<std::string>{});
		for (auto symbolRange : symbols)
		{
			auto rangeTokens = ParserUtils::getNumbers(symbolRange, '-', std::set<int>{});
			for (auto x = rangeTokens[0]; x <= rangeTokens[1]; x++)
			{
				flagTypes[flagType][flagTypeID].push_back(x);
			}
		}
		for (auto& cGroup : colourGroupStrings)
		{
			flagTypeColours[flagType][flagTypeID].push_back(cGroup);
		}
		//auto colour = ParserUtils::getNumbers(tokens[i], ',', std::set<int>{});
		//colourGroups[tokens[0]].push_back(Colour(colour));
		//flagTypes[tokens[0]].push
	}

}
