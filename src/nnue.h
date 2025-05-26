#pragma once

#include "chess/core.h"

#include <cstdint>
#include <cmath>
#include <array>
#include <algorithm>

namespace episteme::nn {
    struct Accumulator {
        std::array<int16_t, 1024> stm;
        std::array<int16_t, 1024> ntm;
    };

    class NNUE {
        public:
            Accumulator l0Propagate(const std::array<Piece, 64>& mailbox) const;
            Accumulator l0Activate(const Accumulator& accum) const;
            int16_t l1Propagate(const Accumulator& accum) const; 

        private:
            std::array<std::array<std::array<int16_t, 768>, 1024>, 2> l0Weights;
            std::array<std::array<int16_t, 1024>, 2> l0Biases;
            std::array<std::array<int16_t, 1024>, 2> l1Weights;
            int16_t l1Bias;
    };
}