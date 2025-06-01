#pragma once

#include "chess/core.h"

#include <cstdint>
#include <cmath>
#include <array>
#include <algorithm>
#include <cstring>
#include <immintrin.h>

namespace episteme::nn {
    struct Accumulator {
        alignas(32) std::array<int16_t, 1024> stm = {};
        alignas(32) std::array<int16_t, 1024> ntm = {};
    };

    class NNUE {
        public:
            Accumulator reset_accumulator(const std::array<Piece, 64>& mailbox) const;
            int32_t l1_forward(const Accumulator& accum) const; 

        private:
            using L0Weights = std::array<std::array<int16_t, 1024>, 768>;
            using L0Biases = std::array<int16_t, 1024>;
            using L1Weights = std::array<std::array<int16_t, 1024>, 2>;
            using L1Bias = int16_t;

            alignas(32) L0Weights l0_weights;
            alignas(32) L0Biases l0_biases;
            alignas(32) L1Weights l1_weights;
            alignas(32) L1Bias l1_bias;
    };
}