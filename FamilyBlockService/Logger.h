#pragma once
#include <fstream>
#include <string>

inline void log(const std::string& msg) {
    std::ofstream f("C:\\FamilyBlockService\\debug.log", std::ios::app);
    f << msg << "\n";
}
