#include "evaluate.h"

#ifndef EVALFILE
    #define EVALFILE "src/episteme_test_net.bin"
#endif

INCBIN(NNUE, EVALFILE);

namespace episteme {
    using namespace nn;

    const NNUE* nnue = reinterpret_cast<const NNUE*>(gNNUEData);

    int32_t evaluate(Position position) {
        Accumulator accum = {
            .stm = {},
            .ntm = {}
        };
        accum = nnue->l0Propagate(position.mailboxAll());
        accum = nnue->l0Activate(accum);
        int16_t out = nnue->l1Propagate(accum);
        return out;
    }
}