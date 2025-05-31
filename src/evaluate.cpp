#include "evaluate.h"

#ifndef EVALFILE
    #define EVALFILE "./episteme_test_net.bin"
#endif

INCBIN(NNUE, EVALFILE);

namespace episteme {
    using namespace nn;

    const NNUE* nnue = reinterpret_cast<const NNUE*>(gNNUEData);

    int32_t evaluate(Position& position) {
        Accumulator accum = nnue->l0_forward(position.mailbox_all());
        int32_t out = nnue->l1_forward(accum);
        return out;
    }
}
