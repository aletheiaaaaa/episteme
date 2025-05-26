#include "search.h"

namespace episteme::search {
    using namespace std::chrono;

    bool makeMove(Position& position, const Move& move) {
        position.makeMove(move);

        uint64_t kingBB = position.bitboard(pieceTypeIdx(PieceType::King)) & position.bitboard(colorIdx(position.NTM()) + position.COLOR_OFFSET);
        return !isSquareAttacked(sqFromIdx(std::countr_zero(kingBB)), position, position.STM());
    }

    void unmakeMove(Position& position) {
        position.unmakeMove();
    }

    int32_t Worker::search(Position& position, Line& PV, uint16_t depth, int32_t alpha, int32_t beta, steady_clock::time_point end) {
        if (steady_clock::now() >= end) {
            abort = true;
            return 0;
        };

        if (depth <= 0) {
            return evaluate(position);
        }

        MoveList moveList;
        generateAllMoves(moveList, position);

        int32_t best = -INF;

        Line candidate = {};
        for (size_t i = 0; i < moveList.count(); i++) {
            Move move = moveList.list(i);
            if (!makeMove(position, move)) {
                unmakeMove(position);
                continue;
            }

            int32_t score = -search(position, candidate, depth - 1, -beta, -alpha, end);
            unmakeMove(position);

            if (abort) return 0;
    
            if (score > best) {
                best = score;
            }

            if (score > alpha) {    
                alpha = score;
                PV.updateLine(move, candidate);

                if (score >= beta) {
                    break;
                }
            }
        };  

        return best;
    }

    std::pair<int32_t, Move> Worker::run() {
        int32_t result = 0;

        Line PV = {};
        Position position = parameters.position;
        int32_t time = parameters.time[colorIdx(position.STM())];
        int32_t inc = parameters.inc[colorIdx(position.STM())];

        auto start = steady_clock::now();
        auto end = start + duration_cast<milliseconds>(duration<double>(time / 20 + inc / 2));

        for (int depth = 1; depth < MAX_SEARCH_PLY; depth++) {
            if (steady_clock::now() >= end) break;
            result = search(position, PV, depth, -INF, INF, end);
        };
        
        return {result, PV.moves[0]};
    }
}