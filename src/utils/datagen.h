#pragma once

#include "../engine/chess/movegen.h"
#include "../engine/search/search.h"

#include <iostream>
#include <random>
#include <cstdint>
#include <algorithm>

namespace episteme::datagen {
    constexpr int32_t WIN_SCORE_MIN = 2500;
    constexpr int32_t WIN_PLIES_MIN = 5;
    constexpr int32_t DRAW_SCORE_MAX = 2;
    constexpr int32_t DRAW_PLIES_MIN = 8;
    constexpr int32_t INITIAL_MAX = 300;

    void play_random(Position& position, int32_t num_moves);
    void game_loop(search::Parameters& params);
}