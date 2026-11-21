#include "LevelFile.h"
#include <SFML/System.hpp>
#include <iostream>

int loadLevelDefs(LevelDef* out, int maxOut, const char* path) {
	sf::FileInputStream in;
	if (in.open(path)) {
		auto size = in.getSize();                       
		if (size && *size > 0 && *size < 200000) {
			char* buf = new char[*size + 1];
			auto got = in.read(buf, *size);
			if (got) {
				buf[*got] = 0;
				int n = parseLevels(buf, out, maxOut);
				delete[] buf;
				if (n > 0) return n;
			}
			else delete[] buf;
		}
	}
	std::cout << "assets/levels.json not found or invalid - using the built-in levels.\n";
	return parseLevels(kDefaultLevelsJson, out, maxOut);
}