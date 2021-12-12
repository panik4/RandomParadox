#pragma once
#include "../FastWorldGen/FastWorldGen/FastWorldGenerator.h"
#include "../FormatConverter.h"
#include "Hoi4Parser.h"
#include <filesystem>
#include <experimental/filesystem>
#include "Hoi4ScenarioGenerator.h"
class Hoi4Module
{
public:
	Hoi4Module();
	~Hoi4Module();

	void genHoi(std::string hoi4ModPath, std::string hoi4Path, FastWorldGenerator f, bool useDefaultMap, bool useDefaultStates, bool useDefaultProvinces, ScenarioGenerator& scenGen);

};

