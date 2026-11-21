#pragma once

// Shared campaign progress (stars per level). Index: stars[act][levelIndex], act = 1..4, levelIndex = 0..7.
struct Progress {
	int stars[5][8];
	Progress() { for (int a = 0; a < 5; a++) for (int i = 0; i < 8; i++) stars[a][i] = 0; }
};