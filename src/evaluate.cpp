#include "evaluate.h"

INCBIN(NNUE, EVALFILE);

namespace episteme::eval {
    using namespace nn;

    const NNUE* nnue = reinterpret_cast<const NNUE*>(gNNUEData);

    Accumulator update(const Position& position, const Move& move, Accumulator accum) {
        accum = nnue->update_accumulator(position, move, accum);
        return accum;
    }

    Accumulator reset(const Position& position) {
        Accumulator accum = nnue->reset_accumulator(position);
        return accum;
    }

    int32_t evaluate(Accumulator& accum) {
        int32_t out = nnue->l1_forward(accum);
        return out;
    }
}
