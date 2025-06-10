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
        AllowedCastles allowed_castles;
        bool stm;
        uint8_t half_clock;
        uint32_t full_number;
        Square en_passant = Square::None;
    };
    class Position {
        public:
            Position();

            [[nodiscard]] inline std::array<uint64_t, 8> bitboard_all() const {
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
        
            [[nodiscard]] inline uint8_t half_move_clock() const {
                return state.half_clock; 
            }
        
            [[nodiscard]] inline uint32_t full_move_number() const {
                return state.full_number;
            }
        
            [[nodiscard]] inline Square ep_square() const {
                return state.en_passant;    
            }
        
            [[nodiscard]] inline AllowedCastles::RookPair castling_rights(Color stm) const {
                return state.allowed_castles.rooks[color_idx(stm)];
            }
        
            [[nodiscard]] inline Piece mailbox(int index) const {
                return state.mailbox[index];
            }

            [[nodiscard]] inline std::array<Piece, 64> mailbox_all() const {
                return state.mailbox;
            }

            
            void from_FEN(std::string_view FEN);
            std::string to_fEN() const; 
            void from_start_pos();
            void make_move(const Move& move);
            void unmake_move();

        public:
            static const uint16_t COLOR_OFFSET = 6;

        private:
            std::vector<PositionState> position_history;
            PositionState state;
    };

    Move from_UCI(const Position& position, const std::string& move);
}
