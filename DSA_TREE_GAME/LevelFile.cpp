#include "LevelFile.h"
#include <iostream>
#include <fstream>
#include <cstring>

int loadLevelDefs(LevelDef* out, int maxOut, const char* path) {
    std::ifstream in(path, std::ios::binary);

    if (in.is_open()) {
        in.seekg(0, std::ios::end);
        long size = (long)in.tellg();

        if (size > 0 && size < 200000) {
            char* buf = new char[size + 1];

            in.seekg(0, std::ios::beg);
            in.read(buf, size);

            long got = (long)in.gcount();

            if (got == size) {
                buf[got] = '\0';

                int n = parseLevels(buf, out, maxOut);

                delete[] buf;
                in.close();

                if (n > 0) {
                    return n;
                }
            }
            else {
                delete[] buf;
            }
        }

        in.close();
    }

    std::cout << "assets/levels.json not found or invalid - using the built-in levels.\n";

    return parseLevels(kDefaultLevelsJson, out, maxOut);
}