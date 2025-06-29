#include "perft.h"

namespace episteme {
    Position fen_to_position(std::string_view FEN) {
        Position position;
        position.from_FEN(FEN);
        return position;
    }

    uint64_t perft(Position& position, int32_t depth) {
        if (depth == 0) {
            return 1;
        }

        gen::MoveList move_list;
        gen::generate_all_moves(move_list, position);
        uint64_t move_count = 0;

        for (size_t i = 0; i < move_list.count; i++) {
            position.make_move(move_list.list[i]);
        
            uint64_t king_bb = position.piece_bb(PieceType::King, position.NTM());
        
            if (!gen::is_square_attacked(sq_from_idx(std::countr_zero(king_bb)), position, position.STM())) {
                move_count += perft(position, depth - 1);
            }
        
            position.unmake_move();
        }
        
        return move_count;
    }

    void split_perft(Position& position, int32_t depth) {    
        gen::MoveList move_list;
        gen::generate_all_moves(move_list, position);
    
        uint64_t total = 0;
    
        for (size_t i = 0; i < move_list.count; ++i) {
            Move move = move_list.list[i];
            position.make_move(move);

            uint64_t king_bb = position.piece_bb(PieceType::King, position.NTM());

            bool illegal = gen::is_square_attacked(sq_from_idx(std::countr_zero(king_bb)), position, position.STM());
    
            uint64_t nodes = 0;
            if (!illegal) {
                nodes = perft(position, depth - 1);
                total += nodes;
                std::cout << move.to_string() << ": " << nodes << "\n";
            }
    
            position.unmake_move();
        }
    
        std::cout << "Total nodes: " << total << "\n";
    }
    

    void time_perft(Position& position, int32_t depth) {
        using namespace std::chrono;

        auto start = steady_clock::now();
        uint64_t nodes = perft(position, depth);
        auto end = steady_clock::now();

        auto duration = duration_cast<milliseconds>(end - start);
        double seconds = duration.count() / 1000.0;
        double nps = nodes / (seconds > 0 ? seconds : 1.0);

        std::cout << "info depth " << depth << " nodes " << nodes << " nps" << static_cast<uint64_t>(nps) << std::endl;
    }

}
