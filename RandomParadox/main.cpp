#include "FastWorldGen/FastWorldGen/FastWorldGenerator.h"
#include "FormatConverter.h"
#include "generic/ScenarioGenerator.h"
#include "hoi4/Hoi4Module.h"
using namespace std;
int main() {
	bool useDefaultMap = false;
	bool useDefaultStates = false;
	bool useDefaultProvinces = false;
	
	bool genHoi4 = true;

	if (!Data::getInstance().getConfig("C:\\Users\\Paul\\Documents\\Visual Studio 2017\\Projects\\RandomParadox\\RandomParadox\\FastWorldGen\\config.json"))
	{
		system("pause");
		return -1;
	}
	FastWorldGenerator f;
	Hoi4Module hoi4Mod;
	if (!useDefaultMap)
	{
		f.generateWorld();
	}
	ScenarioGenerator sG(f);
	if (genHoi4)
	{
		hoi4Mod.genHoi(useDefaultMap, useDefaultStates, useDefaultProvinces, sG);
	}
	return 0;
	
}