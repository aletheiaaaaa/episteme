#pragma once

#include "../chess/move.h"
#include "stack.h"

#include <array>
#include <cstdint>
#include <algorithm>

namespace episteme::hist {
    constexpr int MAX_HISTORY = 16384;

    [[nodiscard]] inline int16_t history_bonus(int16_t depth) {
        return static_cast<int16_t>(std::clamp(depth * 300, 0, 2500));
    }

    [[nodiscard]] inline int16_t history_malus(int16_t depth) {
        return -static_cast<int16_t>(std::clamp(depth * 300, 0, 1250));
    }

    struct Entry {
        int16_t value = 0;

        inline void update(int16_t bonus) {
            value += bonus - value * std::abs(bonus) / MAX_HISTORY;
        }
    };

    class Table {
        public:
            [[nodiscard]] inline int16_t get_quiet_hist(Color stm, Move move) {
                return quiet_hist[color_idx(stm)][sq_idx(move.from_square())][sq_idx(move.to_square())].value;
            }

            [[nodiscard]] inline int16_t get_cont_hist(stack::Stack& stack, Piece piece, Move move, int16_t ply) {
                int16_t value = 0;

                if (ply > 0) {
                    Move prev_move = stack[ply - 1].move;
                    Piece prev_piece = stack[ply - 1].piece;

                    if (prev_piece != Piece::None) value += cont_hist[piece_idx(piece)][sq_idx(move.to_square())][piece_idx(prev_piece)][sq_idx(prev_move.to_square())].value;
                }

                return value;
            }

            inline void update_quiet_hist(Color stm, Move move, int16_t bonus) {
                quiet_hist[color_idx(stm)][sq_idx(move.from_square())][sq_idx(move.to_square())].update(bonus);
            }

            inline void update_cont_hist(stack::Stack& stack, Piece piece, Move move, int16_t bonus, int16_t ply) {
                if (ply > 0) {
                    Move prev_move = stack[ply - 1].move;
                    Piece prev_piece = stack[ply - 1].piece;

                    if (prev_piece != Piece::None) cont_hist[piece_idx(piece)][sq_idx(move.to_square())][piece_idx(prev_piece)][sq_idx(prev_move.to_square())].update(bonus);
                }
            }

            inline void reset() {
                quiet_hist = {};
            }

        private:
            std::array<std::array<std::array<Entry, 64>, 64>, 2> quiet_hist{};
            std::array<std::array<std::array<std::array<Entry, 64>, 12>, 64>, 12> cont_hist{};
    };
}