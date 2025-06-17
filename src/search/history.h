#include "../chess/move.h"

#include <array>
#include <cstdint>
#include <algorithm>

namespace episteme::hist {
    constexpr int MAX_HISTORY = 16384;

    [[nodiscard]] inline int16_t history_bonus(int16_t depth) {
        return static_cast<int16_t>(std::clamp(depth * depth * 20, 0, 2500));
    }

    struct Entry {
        int16_t value = 0;

        inline void update(int16_t bonus) {
            value += bonus - value * std::abs(bonus) / MAX_HISTORY;
        }
    };

    class Table {
        public:
            [[nodiscard]] inline Entry get_butterfly(Color stm, Move move) {
                return butterfly[color_idx(stm)][sq_idx(move.from_square())][sq_idx(move.to_square())];
            }

            inline void update_butterfly(Color stm, Move move, int16_t bonus) {
                butterfly[color_idx(stm)][sq_idx(move.from_square())][sq_idx(move.to_square())].update(bonus);
            }

            inline void reset() {
                butterfly = {};
            }

        private:
            std::array<std::array<std::array<Entry, 64>, 64>, 2> butterfly = {};
    };
}