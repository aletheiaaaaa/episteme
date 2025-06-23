#pragma once

#include "move.h"
#include "zobrist.h"

#include <array>
#include <vector>
#include <string>
#include <ranges>
#include <cstdint>
#include <cstdlib>

namespace episteme {
    static constexpr std::array<Piece, 64> empty_mailbox() {
        std::array<Piece, 64> mailbox{};
        mailbox.fill(Piece::None);
        return mailbox;
    }

    struct PositionState {
        std::array<uint64_t, 8> bitboards{};
        std::array<Piece, 64> mailbox = empty_mailbox();
    
        AllowedCastles allowed_castles{
            .rooks{
                {{.kingside = Square::None, .queenside = Square::None},
                {.kingside = Square::None, .queenside = Square::None}}
            }
        };
    
        bool stm = color_idx(Color::White);
        uint8_t half_move_clock = 0;
        uint32_t full_move_number = 0;
        Square ep_square = Square::None;

        uint64_t hash = 0;
    };

    class Position {
        public:
            Position();

            [[nodiscard]] inline std::array<uint64_t, 8> bitboards_all() const {
                return state.bitboards;
            }

            [[nodiscard]] inline uint64_t bitboard(int index) const {
                return state.bitboards[index];
            }
        
            [[nodiscard]] inline Color STM() const {
                return static_cast<Color>(state.stm);
            }
        
            [[nodiscard]] inline Color NTM() const {
                return static_cast<Color>(!state.stm);
            }
        
            [[nodiscard]] inline uint8_t half_move_clock() const {
                return state.half_move_clock; 
            }
        
            [[nodiscard]] inline uint32_t full_move_number() const {
                return state.full_move_number;
            }
        
            [[nodiscard]] inline Square ep_square() const {
                return state.ep_square;    
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

            [[nodiscard]] inline uint64_t zobrist() const {
                return state.hash;
            }

            void from_FEN(std::string_view FEN);
            void from_startpos();

            void make_move(const Move& move);
            void make_null();
            void unmake_move();

            bool is_threefold();

            std::string to_FEN() const; 
            uint64_t explicit_zobrist();
        public:
            static const uint16_t COLOR_OFFSET = 6;

        private:
            std::vector<PositionState> position_history;
            PositionState state;
    };

    Move from_UCI(const Position& position, const std::string& move);
}
