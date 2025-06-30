#include "datagen.h"

namespace episteme::datagen {
    bool play_random(Position& position, int32_t num_moves, uint64_t seed) {
        MoveList move_list;
        generate_all_moves(move_list, position);

        std::mt19937 gen(seed);
        std::uniform_int_distribution dist(1, 100);

        if (move_list.list.empty()) return false;

        if (num_moves == 0) {
            std::cout << "info string genfens " << position.to_FEN() << std::endl;
            return true;
        }

        Move move{};
        while (!move.data()) {
            int r = dist(gen) % 100;
            PieceType random_type = (
                r < 35 ? PieceType::Pawn : r < 50 ? PieceType::Knight : r < 65 ? PieceType::Bishop : r < 80 ? PieceType::Queen: r < 95 ? PieceType::King : PieceType::Rook
            );

            MoveList random_moves;
            for (size_t i = 0; i < move_list.count; i++) {
                position.make_move(move_list.list[i]);
                if (in_check(position, position.STM())) {
                    position.unmake_move();
                    continue;
                }

                if (piece_type(position.mailbox(move.from_square())) == random_type) {
                    random_moves.add(move_list.list[i]);
                }
                position.unmake_move();
            }

            if (!move_list.list.empty()) {
                move = random_moves.list[dist(gen) % random_moves.count];
            }
        }

        position.make_move(move);
        return play_random(position, num_moves - 1, seed);
    }    
}