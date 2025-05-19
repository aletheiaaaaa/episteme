#include "search.h"

namespace episteme {
    bool makeMove(Position& position, const Move& move) {
        position.makeMove(move);

        uint64_t kingBB = position.bitboard(pieceTypeIdx(PieceType::King)) & position.bitboard(colorIdx(position.NTM()) + position.COLOR_OFFSET);
        return !isSquareAttacked(sqFromIdx(std::countr_zero(kingBB)), position, position.STM());
    }

    void unmakeMove(Position& position) {
        position.unmakeMove();
    }

    int32_t search(Position& position, uint16_t depth, int32_t alpha, int32_t beta) {
        if (depth <= 0) {
            return evaluate(position);
        }

        MoveList moveList;
        generateAllMoves(moveList, position);

        int32_t best = -99999999;

        for (size_t i = 0; i < moveList.count(); i++) {
            Move move = moveList.list(i);

            if (!makeMove(position, move)) {
                unmakeMove(position);
                continue;
            }

            int32_t score = -search(position, depth - 1, -beta, -alpha);
            unmakeMove(position);

            if (score > best) {
                best = score;
            }

            if (score > alpha) {    
                alpha = score;

                if (score >= beta) {
                    break;
                }
            }
        };  

        return best;
    }
}