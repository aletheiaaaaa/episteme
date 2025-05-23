#include "evaluate.h"

INCBIN(NNUE, "./episteme_test_net.bin");;

namespace episteme {
    using namespace nn;

    NNUE nnue = *reinterpret_cast<const NNUE*>(gNNUEData);

    int32_t evaluate(Position position) {
        Accumulator accum = {
            .stm = {},
            .ntm = {}
        };
        accum = nnue.l0Propagate(position.mailboxAll());
        accum = nnue.l0Activate(accum);
        int16_t out = nnue.l1Propagate(accum);
        return out;
    }
}