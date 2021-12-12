#include "FastWorldGen/FastWorldGen/FastWorldGenerator.h"
#include "FormatConverter.h"
#include <filesystem>
#include <experimental/filesystem>
#include "generic/ScenarioGenerator.h"
#include "hoi4/Hoi4Module.h"
using namespace std;
int main() {
	string modPath = "pdoxMod\\";
	string hoiPath = "D:\\Steam\\steamapps\\common\\Hearts of Iron IV\\";
	bool useDefaultMap = true;
	bool useDefaultStates = true;
	bool useDefaultProvinces = true;

	bool genHoi4 = true;

	FastWorldGenerator f;
	Hoi4Module hoi4Mod;
	if (!useDefaultMap)
	{
		f.generateWorld();
	}
	ScenarioGenerator sG(f);
	if (genHoi4)
	{
		hoi4Mod.genHoi(modPath, hoiPath, f, useDefaultMap, useDefaultStates, useDefaultProvinces, sG);
	}

	
}