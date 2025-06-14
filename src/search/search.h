#pragma once

#include "../chess/movegen.h"
#include "../evaluation/evaluate.h"
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
    extern ScoredList generate_scored_targets(const Position& position, F generator, bool include_quiets, const std::optional<tt::TTEntry>& tt_entry = std::nullopt);

    inline ScoredList generate_scored_moves(const Position& position, const tt::TTEntry& tt_entry) {
        return generate_scored_targets(position, generate_all_moves, true, tt_entry);
    }

    inline ScoredList generate_scored_captures(const Position& position) {
        return generate_scored_targets(position, generate_all_captures, false);
    }

    void pick_move(ScoredList& scored_list, int start);

    // BEGIN SEARCH //

    struct Parameters {
        std::array<int32_t, 2> time = {};
        std::array<int32_t, 2> inc = {};

        int16_t depth = 0;
        uint64_t nodes = 0;

        Position position;
    };

    struct SearchLimits {
        std::optional<steady_clock::time_point> end;
        std::optional<uint64_t> max_nodes;
    
        bool time_exceeded() const {
            return end && steady_clock::now() >= *end;
        }
    
        bool node_exceeded(uint64_t current_nodes) const {
            return max_nodes && current_nodes >= *max_nodes;
        }
    };    

    struct Config {
        Parameters params = {};
        uint32_t hash_size = 32;
        uint16_t num_threads = 1;
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

    struct ThreadReport {
        int16_t depth;
        int64_t time;
        uint64_t nodes;
        int64_t nps;
        int32_t score;
        Line line;
    };

    class Thread {
        public:
            Thread(tt::TTable& ttable) : ttable(ttable) {};

            int32_t search(Position& position, Line& PV, int16_t depth, int16_t ply, int32_t alpha, int32_t beta, SearchLimits limits);
            int32_t quiesce(Position& position, int16_t ply, int32_t alpha, int32_t beta, SearchLimits limits);
            ThreadReport run(const Parameters& params);
            void bench(int depth);
        private:
            nn::Accumulator accumulator;
            std::vector<nn::Accumulator> accum_history;

            tt::TTable& ttable;
            uint64_t nodes;
    };

    class Instance {
        public:
            Instance(Config& cfg) : ttable(cfg.hash_size), params(cfg.params), thread(ttable) {};

            void run();
            void bench(int depth);
        private:
            tt::TTable ttable;
            Parameters params;

            Thread thread;
    };

    bool is_legal(const Position& position);
}
