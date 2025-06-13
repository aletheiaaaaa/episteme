#pragma once

#include "chess/move.h"

#include <cstdint>
#include <vector>
#include <random>
#include <bit>

namespace episteme::tt {
    enum class NodeType : uint8_t {
        PVNode, AllNode, CutNode, None
    };

    struct TTEntry {
        uint64_t hash = 0; 
        Move move = {};
        int32_t score = 0;
        uint8_t depth = 0;
        NodeType node_type = NodeType::None;
    };

    class TTable {
        public:
            TTable(uint32_t size);

            [[nodiscard]] inline uint64_t table_index(uint64_t hash) {
                return static_cast<uint64_t>((static_cast<unsigned __int128>(hash) * static_cast<unsigned __int128>(ttable.size())) >> 64);
            }

            [[nodiscard]] inline TTEntry probe(uint64_t hash) {
                uint64_t index = table_index(hash);
                TTEntry entry;

                if (ttable[index].hash == hash) entry = ttable[index];

                return entry;
            }

            inline void clear() {
                ttable.clear();
            }

            inline void add(TTEntry tt_entry) {
                uint64_t index = table_index(tt_entry.hash);
                ttable[index] = tt_entry;
            };

        private:
            std::vector<TTEntry> ttable;
    };
}