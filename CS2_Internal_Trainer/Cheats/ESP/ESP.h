#pragma once
#include "MyD3D_Utils.h"
#include "MyD3D11.h"
#include "LocalPlayer.h"
#include "Entity.h"
#include "ConsoleMenu.h"
#include "PatternScan.h"
#include <vector>

extern MyD3D11 g_myD3d11;

namespace ESP
{
	bool Start(const std::vector<Entity>& pTargets);
};