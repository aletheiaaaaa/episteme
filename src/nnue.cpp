#include "nnue.h"

namespace valhalla {

    Accumulator NNUE::l0Propagate(const std::array<Piece, 64>& mailbox) {
        Accumulator out = {
            .stm = {},
            .ntm = {}
        };
        for (int i = 0; i < 64; i++) {
            int idx = pieceSquare(mailbox[i], sqFromIdx(i));
            if (idx == -1) continue;

            for (int j = 0; j < 1024; j++) {
                out.stm[j] += l0Weights[0][j][idx];
                out.ntm[j] += l0Weights[1][j][idx];
            }
        }
        
        for (int i = 0; i < 1024; i++) {
            out.stm[i] += l0Biases[0][i];
            out.ntm[i] += l0Biases[1][i];
        }

        return out;
    }

    Accumulator NNUE::l0Activate(const Accumulator& accum) {
        Accumulator out = {
            .stm = {},
            .ntm = {}
        };
        for (int i = 0; i < 1024; i++) {
            out.stm[i] = pow(std::clamp(static_cast<int>(accum.stm[i]), 0, 1), 2);
            out.ntm[i] = pow(std::clamp(static_cast<int>(accum.ntm[i]), 0, 1), 2);
        }
        return out;
    }

    int16_t NNUE::l1Propagate(const Accumulator& accum) {
        int16_t out = 0;
        for (int i = 0; i < 1024; i++) {
            out += l1Weights[0][i] * accum.stm[i];
            out += l1Weights[1][i] * accum.ntm[i];
        }
        out += l1Bias;
        return out;
    }

}