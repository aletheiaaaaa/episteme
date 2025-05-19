#pragma once

#include "movegen.h"
#include <string>
#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>

namespace episteme {
    Position fen2Position(std::string_view FEN);
    uint64_t perft(Position &position, int32_t depth);
    void splitPerft(Position &position, int32_t depth);
    void timePerft(Position& position, int32_t depth);
}