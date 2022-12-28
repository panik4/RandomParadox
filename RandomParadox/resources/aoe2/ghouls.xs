

const int numUnitTechs = 5;
int unitTechs = 0;
int specTechs = 0;
int pKillsID = 0;
rule addGold
    active
    minInterval 5
    maxInterval 5
    group chatGroup
{
	// get diff in kills and rewards
	int playerkillsNow = 0;
	for (i = 1; <= 1)
	{
		playerkillsNow = xsPlayerAttribute(i, cAttributeKills);
    	xsChatData("pkillnow = "+playerkillsNow);
		int pkillsbuffered = xsArrayGetInt(pKillsID, i);
    	xsChatData("pkillsbuffered = "+pkillsbuffered);
		int killsRewarded = playerkillsNow - pkillsbuffered;
    	xsChatData("killsRewarded = "+killsRewarded);
		int gold = xsPlayerAttribute(i, cAttributeGold);
		xsSetPlayerAttribute(i, cAttributeGold, gold+10*killsRewarded);
		xsArraySetInt(pKillsID, i, playerkillsNow);
	}
}


bool isImportantTech(int ID = 0){
	int techID = 0;
	for (x = 0; < numUnitTechs)
	{
		techID = xsArrayGetInt(unitTechs, x);
		if(techID==ID)
			return (true);
	}
	for (x = 0; < 1)
	{
		techID = xsArrayGetInt(specTechs, x);
		if(techID==ID)
			return (true);
	}
	return (false);
}

// reenable some techs
rule modTechsInit
    active
    minInterval 1
    maxInterval 1
    group chatGroup
{
	// disable all techs
	for (i = 0; <= 900)
    {
		if(isImportantTech(i) == false)
			xsEffectAmount(cEnableTech, i, cAttributeDisable, 0, -1);
    }
	xsDisableSelf();
}




rule limitRange
    active
    minInterval 2
    maxInterval 2
    group unit
{
xsEffectAmount(cSetAttribute, 492, cMaxRange, 8, 1);
}

rule spawnUnits
    active
    minInterval 5
    maxInterval 5
    group unit
{
	xsSetTriggerVariable(1, 1);
}

void main() {

	unitTechs = xsArrayCreateInt(numUnitTechs, 0, "unitTechs");
	specTechs = xsArrayCreateInt(1, 0, "specTechs");
	xsArraySetInt(specTechs, 0, 101);
	xsArraySetInt(unitTechs, 0, 199);
	xsArraySetInt(unitTechs, 1, 211);
	xsArraySetInt(unitTechs, 2, 67);
	xsArraySetInt(unitTechs, 3, 74);
	xsArraySetInt(unitTechs, 4, 81);

	pKillsID = xsArrayCreateInt(8, 1, "pKillsArray");
	int techID = 0;
	for (i = 0; < numUnitTechs)
	{
		techID = xsArrayGetInt(unitTechs, i);
    	xsChatData("techID = "+techID);
		xsEffectAmount(cModifyTech, techID, cAttrSetTime, 1, -1);
	}

	//xsEffectAmount(cModifyTech, 47, 4, 103, -1);
	for (i = 0; <= 900)
    {
        xsEffectAmount(cModifyTech, i, cAttrSetStacking, 1, -1);
        xsEffectAmount(cEnableObject, i, cAttributeDisable, 1, -1);
    }


	xsEffectAmount(cModResource, cAttributeStone, cSetAttribute, 5000, -1);
	xsEffectAmount(cModResource, cAttributeStone, cSetAttribute, 5000, -1);
}
