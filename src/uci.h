#pragma once

#include "chess/position.h"
#include "chess/perft.h"
#include "search.h"

#include <string>
#include <sstream>
#include <iostream>
#include <array>

namespace episteme {
    int parse(const std::string& cmd, search::Parameters& params);

    auto uci();
    // auto setOption(const std::string& args, search::Parameters params);
    auto isready();
    auto position(const std::string& args, search::Parameters& params);
    auto go(const std::string& args, search::Parameters& params);
    auto ucinewgame(search::Parameters& params);
    auto bench(const std::string& args);
    auto perft(const std::string& args, search::Parameters& params);
}