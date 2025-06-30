#include "datagen.h"

namespace episteme::datagen {
    bool play_random(Position& position, int32_t num_moves) {
        MoveList move_list;
        generate_all_moves(move_list, position);

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution dist(1, 100);

        if (!move_list.count) return false;

        if (num_moves == 0) {
            std::cout << "info string genfens " << position.to_FEN() << std::endl;
            return true;
        }

        Move move{};
        while (!move.data()) {
            int r = dist(gen);
            PieceType random_type = (
                r < 25 ? PieceType::Pawn : r < 40 ? PieceType::Knight : r < 55 ? PieceType::Bishop : r < 70 ? PieceType::Queen: r < 85 ? PieceType::King : PieceType::Rook
            );

            MoveList random_moves;
            for (size_t i = 0; i < move_list.count; i++) {
                position.make_move(move_list.list[i]);
                if (in_check(position, position.STM())) {
                    position.unmake_move();
                    continue;
                }
                position.unmake_move();

                if (piece_type(position.mailbox(move_list.list[i].from_square())) == random_type) {
                    random_moves.add(move_list.list[i]);
                }
            }

            if (random_moves.count) {
                move = random_moves.list[dist(gen) % random_moves.count];
            }
        }

        position.make_move(move);
        return play_random(position, num_moves - 1);
    }    
}