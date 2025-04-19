#include "search.h"

namespace valhalla {
    int32_t search(Position& position, uint16_t depth, int32_t alpha, int32_t beta) {
        if (depth <= 0) {
            return evaluate(position);
        }

        MoveList moveList;
        generateAllMoves(moveList, position);

        int32_t best = -99999999;

        for (size_t i = 0; i < moveList.count(); i++) {
            Move move = moveList.list(i);
            position.makeMove(move);
            int32_t score = -search(position, depth - 1, -beta, -alpha);
            position.unmakeMove();

            if (score > best) {
                best = score;
            }

            if (score > alpha) {    
                alpha = score;

                if (score >= beta) {
                    break;
                }
            }
        }
        
        return best;
    }
}