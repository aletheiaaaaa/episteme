#pragma once

#include "../chess/movegen.h"
#include "../evaluation/evaluate.h"
#include "ttable.h"
#include "history.h"

#include <cstdint>
#include <chrono>
#include <algorithm>
#include <iostream>
#include <optional>

namespace episteme::search {
    using namespace std::chrono;

    constexpr int32_t INF = 1048576;
    constexpr int32_t MATE = 1048575;
    constexpr int MAX_SEARCH_PLY = 256;

    // BEGIN MOVEPICKING //

    struct ScoredMove {
        Move move = {};
        int32_t score = 0;
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
            Thread(tt::Table& ttable) : ttable(ttable) {};

            inline void reset_history() {
                history.reset();
            }

            ScoredMove score_move(const Position& position, const Move& move, const tt::Entry& tt_entry);

            template<typename F>
            ScoredList generate_scored_targets(const Position& position, F generator, const tt::Entry& tt_entry);

            inline ScoredList generate_scored_moves(const Position& position, const tt::Entry& tt_entry) {
                return generate_scored_targets(position, generate_all_moves, tt_entry);
            }
        
            inline ScoredList generate_scored_captures(const Position& position, const tt::Entry& tt_entry) {
                return generate_scored_targets(position, generate_all_captures, tt_entry);
            }

            int32_t search(Position& position, Line& PV, int16_t depth, int16_t ply, int32_t alpha, int32_t beta, SearchLimits limits);
            int32_t quiesce(Position& position, Line& PV, int16_t ply, int32_t alpha, int32_t beta, SearchLimits limits);

            ThreadReport run(const Parameters& params);
            int32_t eval(const Parameters& params);
            void bench(int depth);
        private:
            nn::Accumulator accumulator;
            std::vector<nn::Accumulator> accum_history;

            tt::Table& ttable;
            hist::Table history;

            uint64_t nodes;
    };

    class Instance {
        public:
            Instance(Config& cfg) : ttable(cfg.hash_size), params(cfg.params), thread(ttable) {};

            inline void set_cfg(search::Config& cfg) {
                ttable.resize(cfg.hash_size);
            }

            inline void update_params(search::Parameters& new_params) {
                params = new_params;
            }

            inline void reset_tt() {
                size_t size = ttable.size();
                ttable.reset(size);
            }

            inline void reset_history() {
                thread.reset_history();
            }

            void run();
            void eval(const Parameters& params);
            void bench(int depth);
        private:
            tt::Table ttable;
            Parameters params;

            Thread thread;
    };

    bool in_check(const Position& position, Color color);
}
