#pragma once

#include "chess/movegen.h"
#include "evaluate.h"

#include <cstdint>
#include <chrono>
#include <algorithm>
#include <iostream>
#include <optional>

namespace episteme::search {
    using namespace std::chrono;

    constexpr int32_t INF = 99999999;
    constexpr int MAX_SEARCH_PLY = 256;

    // BEGIN MOVEPICKING //

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

    ScoredList generate_scored_moves(const Position& position);

    void pick_move(ScoredList& scored_list, int start);

    // BEGIN SEARCH //

    struct Parameters {
        std::array<int32_t, 2> time = {};
        std::array<int32_t, 2> inc = {};

        Position position;
    };

    struct Line {
        size_t length = 0;
        std::array<Move, MAX_SEARCH_PLY + 1> moves = {};
        
        inline void clear() {
            length = 0;
        }

        inline void append(Move move) {
            moves[length++] = move;
        }

        inline void append(const Line& line) {
            std::copy(&line.moves[0], &line.moves[line.length], &moves[length]);
            length += line.length;
        }

        inline void update_line(Move move, const Line& line) {
            clear();
            append(move);
            append(line);
        }
    };


    class Worker {
        public:
            int32_t search(Position& position, Line& PV, uint16_t depth, int32_t alpha, int32_t beta, std::optional<steady_clock::time_point> end);
            std::pair<int32_t, Move> run(const Parameters& params);
            void bench();
        private:
            Parameters parameters;
            uint64_t nodes;
    };

    bool isLegal(const Position& position);
}
