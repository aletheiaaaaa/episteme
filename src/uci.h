#include "chess/position.h"

#include <string>
#include <sstream>
#include <iostream>
#include <array>

namespace episteme {
    int parse(const std::string& cmd);

    auto uci();
    auto isReady();
    auto position(const std::string& params);
    auto go(const std::string& params);
    auto uciNewGame();
    auto quit();
}