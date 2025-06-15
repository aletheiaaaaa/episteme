#include "../chess/movegen.h"
#include "ttable.h"

#include <cstdint>
#include <algorithm>
#include <iostream>
#include <optional>

namespace episteme {
    struct ScoredMove {
        Move move;
        int mvv_lva = 0;
    };

    class ScoredList {
        public:
            inline void add(const ScoredMove& move) {
                the_list[the_count] = move;
                the_count++;
            }
        
            inline void clear() {
                the_count = 0;
            }
        
            [[nodiscard]] inline uint32_t count() const {
                return the_count;
            }
        
            [[nodiscard]] inline const ScoredMove list(int index) const {
                return the_list[index];
            }

            inline void swap(int src_idx, int dst_idx) {
                std::iter_swap(the_list.begin() + src_idx, the_list.begin() + dst_idx);
            }

        private:
            std::array<ScoredMove, 256> the_list;
            int the_count = 0;
    };

    template<typename F>
    extern ScoredList generate_scored_targets(const Position& position, F generator, bool include_quiets, const std::optional<tt::TTEntry>& tt_entry = std::nullopt);

    inline ScoredList generate_scored_moves(const Position& position, const tt::TTEntry& tt_entry) {
        return generate_scored_targets(position, generate_all_moves, true, tt_entry);
    }

    inline ScoredList generate_scored_captures(const Position& position) {
        return generate_scored_targets(position, generate_all_captures, false);
    }

    void pick_move(ScoredList& scored_list, int start);
}