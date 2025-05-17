#include "./movegen.h"
#include "evaluate.h"

#include <cstdint>

namespace valhalla {
    int32_t search(Position& position, uint16_t depth, int32_t alpha, int32_t beta);
}