// this is a comment
/* this
is a multiline
comment
*/



void femaleSpeed()
{
xsEffectAmount(cSetAttribute, 83, cFoodCost, 5, -1);
xsEffectAmount(cSetAttribute, 293, cFoodCost, 5, -1); //Sets female villager (ID 293) speed (attribute ID 5) to 2 for player 1 (player ID 1)
xsEffectAmount(cSetAttribute, 83, 5, 5, -1);
xsEffectAmount(cSetAttribute, 293, 5, 5, -1); //Sets female villager (ID 293) speed (attribute ID 5) to 2 for player 1 (player ID 1)
}


int p1Techs = 0;


bool checkForNewResearch(int playerID = 0){
	int researched = xsPlayerAttribute(1, cAttributeResearchCount);
	xsChatData("p1Researched prev techs: " + p1Techs);
	xsChatData("p1Researched techs: " + researched);
	if(researched > p1Techs){
		p1Techs = researched;
		xsChatData("p1Researched new tech");
		return (true);
	}
	return (false);
	
}

void researchLoom()
{
	xsResearchTechnology(22, true, false, 1);
}


void addGold()
{
	int gold = xsPlayerAttribute(1, cAttributeGold);
	xsSetPlayerAttribute(1, cAttributeGold, gold+50);
}

int q = 0;
// This rule prints the value of q every 2 seconds.
rule chatTheValueOfA
    active
    minInterval 2
    maxInterval 2
    group chatGroup
{
	//researchLoom();
	checkForNewResearch(1);
	addGold();
    xsChatData("q = "+q);
    q++;
	xsEffectAmount(cSetAttribute, cAttributeGold, cAttributeSet, 500, -1);
	xsEffectAmount(cSetPlayerData, cAttributeGold, cAttributeSet, 500, -1);
}


void main() {
    int a = 10;
    int b = 20;

	p1Techs = xsPlayerAttribute(1, cAttributeResearchCount);
    // the variables cannot be declared by separating them with commas
    // unlike java or python.

    // chats to the screen
    xsChatData("a+b = "+(a+b));

	//researchLoom();
	femaleSpeed();
	//void xsEffectAmount(int effectID, int unitOrTechnologyID, int attributeOrOperation, float value, int playerNumber)
}
