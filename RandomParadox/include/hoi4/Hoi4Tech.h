#pragma once
#include <array>
#include <string>
#include <vector>

namespace Scenario::Hoi4 {

enum class TechEra { Interwar, Buildup, Early };

enum class NavalHullType { Light, Cruiser, Heavy, Carrier, Submarine };

struct Technology {
  std::string name;
  std::string predecessor;
  TechEra era;
};

static std::map<TechEra, std::vector<Technology>> navyTechs = {
    {TechEra::Interwar,
     {{"basic_cruiser_armor_scheme", "", TechEra::Interwar},
      {"basic_battery", "", TechEra::Interwar},
      {"basic_torpedo", "", TechEra::Interwar},
      {"basic_naval_mines", "", TechEra::Interwar},
      {"submarine_mine_laying", "", TechEra::Interwar},
      {"mtg_transport", "", TechEra::Interwar},
      {"smoke_generator", "", TechEra::Interwar},
      {"basic_depth_charges", "", TechEra::Interwar},
      {"sonar", "", TechEra::Interwar}

     }},
    {TechEra::Buildup,
     {{"basic_heavy_armor_scheme", "basic_cruiser_armor_scheme",
       TechEra::Buildup},
      {"basic_light_battery", "basic_battery", TechEra::Buildup},
      {"basic_medium_battery", "basic_battery", TechEra::Buildup},
      {"basic_heavy_battery", "basic_battery", TechEra::Buildup},
      {"magnetic_detonator", "basic_torpedo", TechEra::Buildup},
      {"improved_ship_torpedo_launcher", "basic_torpedo", TechEra::Buildup},
      {"damage_control_1", "", TechEra::Buildup},
      {"fire_control_methods_1", "", TechEra::Buildup},
      {"improved_depth_charges", "basic_depth_charges", TechEra::Buildup},
      {"improved_sonar", "sonar", TechEra::Buildup}

     }},
    {TechEra::Early,
     {{"basic_light_shell", "basic_light_battery", TechEra::Buildup},
      {"basic_medium_shell", "basic_medium_battery", TechEra::Buildup},
      {"basic_heavy_shell", "basic_heavy_battery", TechEra::Buildup}

     }}};
static std::map<TechEra, std::vector<Technology>> infantryTechs = {
    {TechEra::Interwar,
     {

     }},
    {TechEra::Buildup,
     {

     }},
    {TechEra::Early, {}}};
static std::map<TechEra, std::vector<Technology>> armorTechs = {
    {TechEra::Interwar,
     {

     }},
    {TechEra::Buildup,
     {

     }},
    {TechEra::Early, {}}};
static std::map<TechEra, std::vector<Technology>> airTechs = {
    {TechEra::Interwar,
     {

     }},
    {TechEra::Buildup,
     {

     }},
    {TechEra::Early, {}}};

static std::map<TechEra, std::vector<Technology>> industryElectronicTechs = {
    {TechEra::Interwar,
     {

     }},
    {TechEra::Buildup,
     {

     }},
    {TechEra::Early, {}}};
} // namespace Scenario::Hoi4