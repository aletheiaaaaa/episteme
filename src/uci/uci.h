#pragma once

#include "../chess/position.h"
#include "../chess/perft.h"
#include "../search/search.h"

#include <string>
#include <sstream>
#include <iostream>
#include <array>

namespace episteme::uci {
    int parse(const std::string& cmd, search::Config& cfg);

    auto uci();
    auto setoption(const std::string& args, search::Config& cfg);
    auto isready();
    auto position(const std::string& args, search::Config& cfg);
    auto go(const std::string& args, search::Config& cfg);
    auto ucinewgame(search::Config& cfg);
    auto bench(const std::string& args, search::Config& cfg);
    auto perft(const std::string& args, search::Config& cfg);
}