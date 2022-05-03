#pragma once
#include <iostream>
#include <map>
#include <string>
#include <vector>
class NationalFocus {
  static int IDcounter;

public:
  // typedefs
  enum FocusType { attack, ally, generic };
  // constructors/destructors
  NationalFocus();
  NationalFocus(FocusType fType, bool defaultV, std::string source,
                std::string dest, std::vector<int> date);
  ~NationalFocus();
  // member variables
  int ID;
  int stepID;
  int chainID;
  // define FocusType;
  FocusType fType;
  std::string sourceTag;
  std::string destTag;
  bool defaultV;

  // containers
  std::vector<int> date;
  std::vector<int> position;
  std::vector<int> precedingFoci;
  std::vector<int> andFoci;
  std::vector<int> xorFoci;
  std::vector<int> orFoci;
  std::vector<std::string> available;
  std::vector<std::string> bypasses;
  std::vector<std::string> completionRewards; // e.g. set_country_flag
  static std::map<std::string, NationalFocus::FocusType> typeMapping;
  // map completion keys to completion values
  static std::map<std::string, std::string> availableMap;
  static std::map<std::string, std::string> bypassMap;
  static std::map<std::string, std::string> rewardMap;

  // map keys to values
  static void buildMaps();
  // operators
  friend std::ostream &operator<<(std::ostream &os, const NationalFocus &focus);
  bool operator==(const NationalFocus &right) const { return ID == right.ID; }
};
