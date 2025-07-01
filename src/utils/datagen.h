#pragma once

#include "../engine/chess/movegen.h"
#include "../engine/search/search.h"

#include <iostream>
#include <random>
#include <cstdint>

namespace episteme::datagen {
    struct Parameters {
        uint32_t soft_limit = 0;
        uint32_t hard_limit = 0;
        int32_t num_games = 100000;
        std::string out_file;
    };

    bool play_random(Position& position, int32_t num_moves);
    void game_loop(const Parameters& params);
}