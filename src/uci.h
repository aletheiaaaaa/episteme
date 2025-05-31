#pragma once

#include "chess/position.h"
#include "search.h"

#include <string>
#include <sstream>
#include <iostream>
#include <array>

namespace episteme {
    int parse(const std::string& cmd, search::Parameters& params);

    auto uci();
    // auto setOption(const std::string& args, search::Parameters params);
    auto isReady();
    auto position(const std::string& args, search::Parameters& params);
    auto go(const std::string& args, search::Parameters& params);
    auto uciNewGame(search::Parameters& params);
}