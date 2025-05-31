#include "nnue.h"

namespace episteme::nn {
    Accumulator NNUE::l0_forward(const std::array<Piece, 64>& mailbox) const {
        Accumulator out;

        for (uint8_t i = 0; i < 64; i++) {
            int stm = piecesquare(mailbox[i], sq_from_idx(i), false);
            int ntm = piecesquare(mailbox[i], sq_from_idx(i ^ 56), true);

            if (stm != -1) {
                for (int j = 0; j < 1024; j++) {
                    out.stm[j] += l0_weights[stm][j];
                }
            }
            if (ntm != -1) {
                for (int j = 0; j < 1024; j++) {
                    out.ntm[j] += l0_weights[stm][j];
                }
            }
        }
        
        for (int i = 0; i < 1024; i++) {
            out.stm[i] += l0_biases[i];
            out.ntm[i] += l0_biases[i];
        }

        for (int i = 0; i < 1024; i++) {
            int stmClipped = std::clamp(static_cast<int>(out.stm[i]), 0, 255);
            int ntmClipped = std::clamp(static_cast<int>(out.ntm[i]), 0, 255);
            out.stm[i] = stmClipped * stmClipped;
            out.ntm[i] = ntmClipped * ntmClipped;
        }

        return out;
    }

    int16_t NNUE::l1_forward(const Accumulator& accum) const {
        int16_t out = 0;
        for (int i = 0; i < 1024; i++) {
            out += l1_weights[0][i] * accum.stm[i];
            out += l1_weights[1][i] * accum.ntm[i];
        }
        out += l1_bias;
        return out;
    }
}