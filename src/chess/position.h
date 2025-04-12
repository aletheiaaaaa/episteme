#pragma once

#include "move.h"
#include <array>
#include <vector>
#include <string>
#include <ranges>
#include <cstdint>
#include <cstdlib>

namespace valhalla {
    struct PositionState {
        std::array<uint64_t, 8> bitboards;
        std::array<Piece, 64> theMailbox;
        AllowedCastles allowedCastles;
        bool stm;
        uint8_t halfClock;
        uint32_t fullNumber;
        Square enPassant = Square::None;
    };
    class Position {
        public:
            Position();

            [[nodiscard]] inline uint64_t bitboard(int index) const {
                return bitboards[index];
            }
        
            [[nodiscard]] inline Color STM() const {
                return static_cast<Color>(stm);
            }
        
            [[nodiscard]] inline Color nSTM() const {
                return static_cast<Color>(!stm);
            }
        
            [[nodiscard]] inline uint8_t halfMoveClock() const {
                return halfClock; 
            }
        
            [[nodiscard]] inline uint32_t fullMoveNumber() const {
                return fullNumber;
            }
        
            [[nodiscard]] inline Square epSquare() const {
                return enPassant;    
            }
        
            [[nodiscard]] inline AllowedCastles castlingRights() const {
                return allowedCastles;
            }
        
            [[nodiscard]] inline  std::array<Piece, 64> mailbox() const {
                return theMailbox;
            }
                
            void fromFEN(std::string_view FEN);
            std::string toFEN() const; 
            void fromStartPos();
            void makeMove(const Move &move);
            void unmakeMove();

        public:
            static const uint16_t COLOR_OFFSET = 6;

        private:
            std::array<uint64_t, 8> bitboards;
            std::array<Piece, 64> theMailbox;
            std::vector<PositionState> positionHistory;
            AllowedCastles allowedCastles;
            bool stm;
            uint8_t halfClock;
            uint32_t fullNumber;
            Square enPassant;
    };
}
