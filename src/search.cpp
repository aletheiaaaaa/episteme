#include "search.h"
#include "bench.h"

namespace episteme::search {
    using namespace std::chrono;

    bool isLegal(const Position& position) {
        uint64_t kingBB = position.bitboard(pieceTypeIdx(PieceType::King)) & position.bitboard(colorIdx(position.NTM()) + position.COLOR_OFFSET);
        return !isSquareAttacked(sqFromIdx(std::countr_zero(kingBB)), position, position.STM());
    };

    int32_t Worker::search(Position& position, Line& PV, uint16_t depth, int32_t alpha, int32_t beta, std::optional<steady_clock::time_point> end) {
        if (end && steady_clock::now() >= *end) return 0;

        if (depth <= 0) {
            return evaluate(position);
        }

        MoveList moveList;
        generateAllMoves(moveList, position);

        int32_t best = -INF;

        Line candidate = {};
        for (size_t i = 0; i < moveList.count(); i++) {
            Move move = moveList.list(i);
            position.makeMove(move);
            if (!isLegal(position)) {
                position.unmakeMove();
                continue;
            }

            nodes++;
            int32_t score = -search(position, candidate, depth - 1, -beta, -alpha, end);
            position.unmakeMove();

            if (end && steady_clock::now() >= *end) return 0;
    
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

    std::pair<int32_t, Move> Worker::run(const Parameters& params) {
        int32_t result = -1;

        Line PV = {};
        Position position = params.position;
        int32_t time = params.time[colorIdx(position.STM())];
        int32_t inc = params.inc[colorIdx(position.STM())];

        auto start = steady_clock::now();
        auto end = start + milliseconds(time / 20 + inc / 2);

        for (int depth = 1; depth < MAX_SEARCH_PLY; depth++) {
            if (steady_clock::now() >= end) break;
            result = search(position, PV, depth, -INF, INF, end);
        };
        
        return {result, PV.moves[0]};
    }

    void Worker::bench() {
        uint64_t total = 0;
        milliseconds elapsed = 0ms;
        for (std::string fen : fens) {
            Line PV = {};
            Position position;
            position.fromFEN(fen);
            nodes = 0;

            auto start = steady_clock::now();
            int32_t _ = search(position, PV, 3, -INF, INF, std::nullopt);
            auto end = steady_clock::now();

            elapsed += duration_cast<milliseconds>(end - start);
            total += nodes;
        }
        
        std::cout << "nodes " << total << " nps " << 1000 * total / elapsed.count() << std::endl;
    }
}