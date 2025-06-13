#pragma once

#include "chess/movegen.h"
#include "evaluate.h"
#include "ttable.h"

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

    template<typename F>
    extern ScoredList generate_scored_targets(const Position& position, F generator, bool include_quiets);

    inline ScoredList generate_scored_moves(const Position& position) {
        return generate_scored_targets(position, generate_all_moves, true);
    }

    inline ScoredList generate_scored_captures(const Position& position) {
        return generate_scored_targets(position, generate_all_captures, false);
    }

    void pick_move(ScoredList& scored_list, int start);

    // BEGIN SEARCH //

    struct Parameters {
        std::array<int32_t, 2> time = {};
        std::array<int32_t, 2> inc = {};
        
        Position position;
    };

    struct Config {
        Parameters params = {};
        uint32_t hash_size = 0;
        uint16_t num_threads = 0;
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

    class Thread {
        public:
            Thread(tt::TTable& ttable);

            int32_t search(Position& position, Line& PV, int16_t depth, int32_t alpha, int32_t beta, std::optional<steady_clock::time_point> end);
            int32_t quiesce(Position& position, int32_t alpha, int32_t beta, std::optional<steady_clock::time_point> end);
            std::pair<int32_t, Line> run(const Parameters& params);
            void bench(int depth);
        private:
            nn::Accumulator accumulator;
            std::vector<nn::Accumulator> accum_history;

            tt::TTable& ttable;
            uint64_t nodes;
    };

    class Instance {
        public:
            Instance(Config& cfg);

            void run();
            void bench(int depth);
        private:
            tt::TTable ttable;
            Parameters params;

            Thread thread;
    };

    bool is_legal(const Position& position);
}
