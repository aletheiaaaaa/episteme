#pragma once

#include "core.h"
#include <array>
#include <string>

namespace episteme {
    enum class MoveType : uint16_t {
        Normal, Castling, EnPassant, Promotion, 
        None
    };

    enum class PromoPiece : uint16_t {
        Knight, Bishop, Rook, Queen, 
        None
    };

    class Move {
        public:
            Move();
            Move(Square from_square, Square to_square, MoveType move_type = MoveType::Normal, PromoPiece promo_piece = PromoPiece::None);

            [[nodiscard]] inline Square from_square() const {
                return static_cast<Square>(move_data & 0b111111);
            }
        
            [[nodiscard]] inline Square to_square() const {
                return static_cast<Square>((move_data >> 6) & 0b111111);
            }
        
            [[nodiscard]] inline MoveType move_type() const {
                return static_cast<MoveType>((move_data >> 12) & 0b11);
            }
        
            [[nodiscard]] inline PromoPiece promo_piece() const {
                return static_cast<PromoPiece>((move_data >> 14) & 0b11);
            }
        
            [[nodiscard]] inline PieceType promo_piece_type() const {
                return static_cast<PieceType>(((move_data >> 14) & 0b11) + 1);
            }
        
            std::string to_string() const;
        private:
            uint16_t move_data;
    };
}
