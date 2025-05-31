#include "nnue.h"

namespace episteme::nn {
    Accumulator NNUE::l0Forward(const std::array<Piece, 64>& mailbox) const {
        Accumulator out;

        for (uint8_t i = 0; i < 64; i++) {
            int stm = pieceSquare(mailbox[i], sqFromIdx(i), false);
            int ntm = pieceSquare(mailbox[i], sqFromIdx(i ^ 56), true);

            if (stm != -1) {
                for (int j = 0; j < 1024; j++) {
                    out.stm[j] += l0Weights[stm][j];
                }
            }
            if (ntm != -1) {
                for (int j = 0; j < 1024; j++) {
                    out.ntm[j] += l0Weights[stm][j];
                }
            }
        }
        
        for (int i = 0; i < 1024; i++) {
            out.stm[i] += l0Biases[i];
            out.ntm[i] += l0Biases[i];
        }

        for (int i = 0; i < 1024; i++) {
            int stmClipped = std::clamp(static_cast<int>(out.stm[i]), 0, 255);
            int ntmClipped = std::clamp(static_cast<int>(out.ntm[i]), 0, 255);
            out.stm[i] = stmClipped * stmClipped;
            out.ntm[i] = ntmClipped * ntmClipped;
        }

        return out;
    }

    int16_t NNUE::l1Forward(const Accumulator& accum) const {
        int16_t out = 0;
        for (int i = 0; i < 1024; i++) {
            out += l1Weights[0][i] * accum.stm[i];
            out += l1Weights[1][i] * accum.ntm[i];
        }
        out += l1Bias;
        return out;
    }
}