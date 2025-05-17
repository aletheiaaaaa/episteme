#include "perft.h"

namespace valhalla {
    Position fen2Position(std::string_view FEN) {
        Position position;
        position.fromFEN(FEN);
        return position;
    }

    uint64_t perft(Position& position, int32_t depth) {
        if (depth == 0) {
            return 1;
        }

        MoveList moveList;
        generateAllMoves(moveList, position);
        uint64_t moveCount = 0;

        for (size_t i = 0; i < moveList.count(); i++) {
            position.makeMove(moveList.list(i));
        
            uint64_t kingBB = position.bitboard(pieceTypeIdx(PieceType::King)) & position.bitboard(colorIdx(position.NTM()) + position.COLOR_OFFSET);
                
            if (!isSquareAttacked(sqFromIdx(std::countr_zero(kingBB)), position, position.STM())) {
                moveCount += perft(position, depth - 1);
            }
        
            position.unmakeMove();
        }
        
        return moveCount;
    }

    void splitPerft(Position& position, int32_t depth) {
        if (depth <= 0) {
            std::cerr << "Depth must be greater than 0\n";
            return;
        }
    
        MoveList moveList;
        generateAllMoves(moveList, position);
    
        uint64_t total = 0;
    
        for (size_t i = 0; i < moveList.count(); ++i) {
            Move move = moveList.list(i);
            position.makeMove(move);
    
            uint64_t kingBB = position.bitboard(pieceTypeIdx(PieceType::King)) & position.bitboard(colorIdx(position.NTM()) + position.COLOR_OFFSET);
    
            bool illegal = isSquareAttacked(sqFromIdx(std::countr_zero(kingBB)), position, position.STM());
    
            uint64_t nodes = 0;
            if (!illegal) {
                nodes = perft(position, depth - 1);
                total += nodes;
                std::cout << move.toString() << ": " << nodes << "\n";
            }
    
            position.unmakeMove();
        }
    
        std::cout << "Total nodes: " << total << "\n";
    }
    

    void timePerft(Position& position, int32_t depth) {
        using namespace std::chrono;

        auto start = steady_clock::now();

        uint64_t nodes = perft(position, depth);

        auto end = steady_clock::now();
        auto duration = duration_cast<milliseconds>(end - start);

        double seconds = duration.count() / 1000.0;
        double nps = nodes / (seconds > 0 ? seconds : 1.0);  // avoid divide-by-zero

        std::cout << "Depth: " << depth << " | ";
        std::cout << "Nodes: " << nodes << " | ";
        std::cout << "Time: " << std::fixed << std::setprecision(3) << seconds << " | ";
        std::cout << "NPS: " << static_cast<uint64_t>(nps) << "\n";
    }

}