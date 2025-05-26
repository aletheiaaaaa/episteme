#pragma once

#include "chess/movegen.h"
#include "evaluate.h"

#include <cstdint>
#include <chrono>
#include <algorithm>

namespace episteme::search {
    using namespace std::chrono;

    constexpr int32_t INF = 99999999;
    constexpr int MAX_SEARCH_PLY = 256;

    struct Parameters {
        std::array<int32_t, 2> time = {};
        std::array<int32_t, 2> inc = {};

        Position position;
    };

    struct Line {
        size_t length = 0;
        std::array<Move, MAX_SEARCH_PLY + 1> moves;
        
        inline void clear() {
            length = 0;
        }

        inline void append(Move move) {
            moves[length++] = move;
        }

        inline void append(const Line& line) {
            std::copy(&line.moves[0], &line.moves[line.length], &moves[length]);
            length += line.length;
        }

        inline void updateLine(Move move, const Line& line) {
            clear();
            append(move);
            append(line);
        }
    };

    struct Worker {
        Parameters parameters;

        int32_t search(Position& position, Line& PV, uint16_t depth, int32_t alpha, int32_t beta, steady_clock::time_point end);
        std::pair<int32_t, Move> run();
    };
}
