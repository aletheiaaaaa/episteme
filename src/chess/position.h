#pragma once

#include "move.h"
#include <array>
#include <vector>
#include <string>
#include <ranges>
#include <cstdint>
#include <cstdlib>

namespace episteme {
    struct PositionState {
        std::array<uint64_t, 8> bitboard;
        std::array<Piece, 64> mailbox;
        AllowedCastles allowedCastles;
        bool stm;
        uint8_t halfClock;
        uint32_t fullNumber;
        Square enPassant = Square::None;
    };
    class Position {
        public:
            Position();

            [[nodiscard]] inline std::array<uint64_t, 8> bitboardAll() const {
                return state.bitboard;
            }

            [[nodiscard]] inline uint64_t bitboard(int index) const {
                return state.bitboard[index];
            }
        
            [[nodiscard]] inline Color STM() const {
                return static_cast<Color>(state.stm);
            }
        
            [[nodiscard]] inline Color NTM() const {
                return static_cast<Color>(!state.stm);
            }
        
            [[nodiscard]] inline uint8_t halfMoveClock() const {
                return state.halfClock; 
            }
        
            [[nodiscard]] inline uint32_t fullMoveNumber() const {
                return state.fullNumber;
            }
        
            [[nodiscard]] inline Square epSquare() const {
                return state.enPassant;    
            }
        
            [[nodiscard]] inline AllowedCastles::RookPair castlingRights(Color stm) const {
                return state.allowedCastles.rooks[colorIdx(stm)];
            }
        
            [[nodiscard]] inline Piece mailbox(int index) const {
                return state.mailbox[index];
            }

            [[nodiscard]] inline std::array<Piece, 64> mailboxAll() const {
                return state.mailbox;
            }

            
            void fromFEN(std::string_view FEN);
            std::string toFEN() const; 
            void fromStartPos();
            void makeMove(const Move& move);
            void unmakeMove();

        public:
            static const uint16_t COLOR_OFFSET = 6;

        private:
            std::vector<PositionState> positionHistory;
            PositionState state;
    };

    Move fromUCI(const Position& position, const std::string& move);
}
