#include "move.h"

namespace episteme {
    Move::Move() {
        moveData = 0x0000;
    }

    Move::Move(Square fromSquare, Square toSquare, MoveType moveType, PromoPiece promoPiece) {
        moveData = static_cast<uint16_t>(fromSquare) 
        | (static_cast<uint16_t>(toSquare) << 6) 
        | (static_cast<uint16_t>(moveType) << 12)
        | (static_cast<uint16_t>(promoPiece) << 14);
    }

    std::string Move::toString() const {
        auto squareToString = [](Square sq) -> std::string {
            if (sq == Square::None) return "--";
            int idx = static_cast<int>(sq);
            char file = 'a' + (idx % 8);
            char rank = '1' + (idx / 8);
            return std::string() + file + rank;
        };
    
        std::string moveStr = squareToString(fromSquare()) + squareToString(toSquare());
    
        if (moveType() == MoveType::Promotion) {
            char promoChar;
            switch (promoPieceType()) {
                case PieceType::Knight: promoChar = 'n'; break;
                case PieceType::Bishop: promoChar = 'b'; break;
                case PieceType::Rook:   promoChar = 'r'; break;
                case PieceType::Queen:  promoChar = 'q'; break;
                default: break;
            }
            moveStr += promoChar;
        }
    
        return moveStr;
    }    
}