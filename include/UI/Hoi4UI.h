#pragma once
#include "ArdaUI/ArdaUI.h"
#include "FastWorldGenerator.h"
#include "hoi4/Hoi4Generator.h"

using Hoi4Gen = Rpx::Hoi4::Generator;
namespace Rpx::UI::Hoi4 {
int showHoi4Configure(Fwg::Cfg &cfg, std::shared_ptr<Hoi4Gen> generator);
};