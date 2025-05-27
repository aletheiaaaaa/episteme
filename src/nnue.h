#pragma once

#include "chess/core.h"

#include <cstdint>
#include <cmath>
#include <array>
#include <algorithm>
#include <memory>

namespace episteme::nn {
    struct Accumulator {
        std::array<int32_t, 1024> stm;
        std::array<int32_t, 1024> ntm;
    };

    class NNUE {
        public:
            Accumulator l0Propagate(const std::array<Piece, 64>& mailbox) const;
            Accumulator l0Activate(const Accumulator& accum) const;
            int16_t l1Propagate(const Accumulator& accum) const; 

        private:
            using L0Weights = std::array<std::array<int16_t, 768>, 1024>;
            using L0Biases = std::array<int16_t, 1024>;
            using L1Weights = std::array<std::array<int16_t, 1024>, 2>;
            using L1Bias = int16_t;

            L0Weights l0Weights;
            L0Biases l0Biases;
            L1Weights l1Weights;
            L1Bias l1Bias;
    };
}