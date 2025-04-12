#pragma once

#include "nnue.h"
#include "./chess/position.h"
#include "./utils/incbin.h"

namespace episteme {
    int32_t evaluate(Position position);
}