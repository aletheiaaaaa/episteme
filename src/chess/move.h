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
            Move(Square fromSquare, Square toSquare, MoveType moveType = MoveType::Normal, PromoPiece promoPiece = PromoPiece::None);

            [[nodiscard]] inline Square fromSquare() const {
                return static_cast<Square>(moveData & 0b111111);
            }
        
            [[nodiscard]] inline Square toSquare() const {
                return static_cast<Square>((moveData >> 6) & 0b111111);
            }
        
            [[nodiscard]] inline MoveType moveType() const {
                return static_cast<MoveType>((moveData >> 12) & 0b11);
            }
        
            [[nodiscard]] inline PromoPiece promoPiece() const {
                return static_cast<PromoPiece>((moveData >> 14) & 0b11);
            }
        
            [[nodiscard]] inline PieceType promoPieceType() const {
                return static_cast<PieceType>(((moveData >> 14) & 0b11) + 1);
            }
        
            std::string toString() const;
        private:
            uint16_t moveData;
    };
}