#include "nnue.h"

namespace episteme::nn {
    Accumulator NNUE::l0Propagate(const std::array<Piece, 64>& mailbox) const {
        Accumulator out = {
            .stm = {},
            .ntm = {}
        };

        for (uint8_t i = 0; i < 64; i++) {
            int stm = pieceSquare(mailbox[i], sqFromIdx(i), false);
            int ntm = pieceSquare(mailbox[i], sqFromIdx(i ^ 56), true);

            if (stm != -1) {
                for (int j = 0; j < 1024; j++) {
                    out.stm[j] += l0Weights[j][stm];
                }
            }
            if (ntm != -1) {
                for (int j = 0; j < 1024; j++) {
                    out.ntm[j] += l0Weights[j][ntm];
                }
            }
        }
        
        for (int i = 0; i < 1024; i++) {
            out.stm[i] += l0Biases[i];
            out.ntm[i] += l0Biases[i];
        }

        return out;
    }

    Accumulator NNUE::l0Activate(const Accumulator& accum) const {
        Accumulator out = {
            .stm = {},
            .ntm = {}
        };
        for (int i = 0; i < 1024; i++) {
            out.stm[i] = pow(std::clamp(static_cast<int>(accum.stm[i]), 0, 255), 2);
            out.ntm[i] = pow(std::clamp(static_cast<int>(accum.ntm[i]), 0, 255), 2);
        }
        return out;
    }

    int16_t NNUE::l1Propagate(const Accumulator& accum) const {
        int16_t out = 0;
        for (int i = 0; i < 1024; i++) {
            out += l1Weights[0][i] * accum.stm[i];
            out += l1Weights[1][i] * accum.ntm[i];
        }
        out += l1Bias;
        return out;
    }
}