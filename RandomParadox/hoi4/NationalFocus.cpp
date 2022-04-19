#include "NationalFocus.h"
#include "Hoi4Parser.h"
int NationalFocus::IDcounter = 0;
std::map<std::string, NationalFocus::FocusType> NationalFocus::typeMapping;
NationalFocus::NationalFocus()
{}

NationalFocus::NationalFocus(FocusType fType, bool defaultV, std::string source, std::string dest, std::vector<int> date) : ID{ ID }, fType{ fType }, defaultV{ defaultV }, sourceTag{ source }, destTag{ dest }, date{ date }
{
	this->ID = IDcounter++;
	position = { 5,5 };
}

NationalFocus::~NationalFocus()
{}

void NationalFocus::mapTypes()
{
	auto types = Hoi4Parser::readTypeMap("resources\\hoi4\\ai\\national_focus\\foci.txt");
	for (int i = 0; i < types.size(); i++) {
		typeMapping[types[i]] = (FocusType)i;
	}
}

std::ostream & operator<<(std::ostream & os, const NationalFocus & focus)
{
	auto action = "";
	switch (focus.fType)
	{
	case NationalFocus::FocusType::attack: {
		action = " attacks ";
		break;
	}
	case NationalFocus::FocusType::ally: {
		action = " tries to ally ";
		break;
	}
	case NationalFocus::FocusType::defense: {
		action = " defends ";
		break;

	}
	}
	os << focus.sourceTag << action << focus.destTag << std::endl;
	os << "Alternatives: " << focus.alternativeFoci.size() << "; Preceding Foci: " << focus.precedingFoci.size();
	return os;
}
