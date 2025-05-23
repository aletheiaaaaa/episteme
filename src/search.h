#include "./movegen.h"
#include "evaluate.h"

#include <cstdint>

namespace episteme::search {
    struct Parameters {
        std::array<int32_t, 2> time = {};
        std::array<int32_t, 2> inc = {};

        Position position;
    };

    int32_t main(Position& position, uint16_t depth, int32_t alpha, int32_t beta);
}