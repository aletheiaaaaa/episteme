#pragma once

#include "../engine/chess/movegen.h"

#include <iostream>
#include <random>

namespace episteme::datagen {
    bool play_random(Position& position, int32_t num_moves, uint64_t seed);
}