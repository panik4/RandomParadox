#pragma once
#include <array>
#include <string>
#include <vector>

namespace Scenario::Hoi4 {

enum class TechEra { Interwar, Buildup, Early };

enum class NavalHullType { Light, Cruiser, Heavy, Carrier, Submarine };

struct Module {
  std::string name;
  std::string predecessor;
  TechEra era;
};

static std::map<TechEra, std::vector<Module>> navyTechs = {
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

static std::map<TechEra, std::vector<Module>> airTechs = {
    {TechEra::Interwar,
     {{"early_transport_plane", "", TechEra::Interwar},
      {"early_fighter", "", TechEra::Interwar},
      {"cv_early_fighter", "early_fighter", TechEra::Interwar},
      {"early_bomber", "", TechEra::Interwar},
      {"early_transport_plane", "", TechEra::Interwar},
      {"early_transport_plane", "", TechEra::Interwar},
      {"early_transport_plane", "", TechEra::Interwar},
      {"early_transport_plane", "", TechEra::Interwar},
      {"early_transport_plane", "", TechEra::Interwar},
      {"early_transport_plane", "", TechEra::Interwar},
      {"early_transport_plane", "", TechEra::Interwar}

     }},
    {TechEra::Buildup,
     {{"fighter1", "early_fighter",
       TechEra::Buildup},
      {"cv_fighter1", "fighter1",
       TechEra::Buildup},
      {"CAS1", "early_fighter",
       TechEra::Buildup},
      {"cv_CAS1", "CAS1",
       TechEra::Buildup},
      {"naval_bomber1", "early_fighter",
       TechEra::Buildup},
      {"cv_naval_bomber1", "naval_bomber1",
       TechEra::Buildup},
      {"heavy_fighter1", "early_bomber",
       TechEra::Buildup},
      {"scout_plane1", "early_bomber",
       TechEra::Buildup},
      {"strategic_bomber1", "",
       TechEra::Buildup},
      {"basic_heavy_armor_scheme", "basic_cruiser_armor_scheme",
       TechEra::Buildup},
      {"basic_heavy_armor_scheme", "basic_cruiser_armor_scheme",
       TechEra::Buildup}

     }},
    {TechEra::Early,
     {

     }}};

} // namespace Scenario::Hoi4