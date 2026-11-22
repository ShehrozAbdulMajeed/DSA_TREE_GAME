#pragma once
#include "LevelData.h"

// Reads assets/levels.json 
int loadLevelDefs(LevelDef* out, int maxOut, const char* path = "assets/levels.json");