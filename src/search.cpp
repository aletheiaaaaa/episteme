#include "search.h"
#include "bench.h"

namespace episteme::search {
    using namespace std::chrono;

    void pick_move(ScoredList& scored_list, int start) {
        const ScoredMove& start_move = scored_list.list(start);
        for (int i = start + 1; i <  scored_list.count(); i++)    {
            if (scored_list.list(i).mvv_lva > start_move.mvv_lva) {
                scored_list.swap(start, i);
            }
        }
    }

    template<typename F>
    ScoredList generate_scored_targets(const Position& position, F generator, bool include_quiets) {
        MoveList move_list;
        generator(move_list, position);
        ScoredList scored_list;

        for (size_t i = 0; i < move_list.count(); i++) {
            Move move = move_list.list(i);
            PieceType src = piece_type(position.mailbox(sq_idx(move.from_square())));
            PieceType dst = piece_type(position.mailbox(sq_idx(move.to_square())));
            int src_val;
            int dst_val;

            bool is_capture = !include_quiets || dst != PieceType::None;

            src_val = piece_vals[piece_type_idx(src)];
            if(is_capture) {
                dst_val = move.move_type() == MoveType::EnPassant ? piece_vals[piece_type_idx(PieceType::Pawn)] : piece_vals[piece_type_idx(dst)];
            } else {
                dst_val = 0;
            }

            int mvv_lva = (dst_val) ? dst_val * 10 - src_val : 0;

            scored_list.add({
                .move = move,
                .mvv_lva = mvv_lva
            });
        }

        return scored_list;
    }

    bool is_legal(const Position& position) {
        uint64_t kingBB = position.bitboard(piece_type_idx(PieceType::King)) & position.bitboard(color_idx(position.NTM()) + position.COLOR_OFFSET);
        return !is_square_attacked(sq_from_idx(std::countr_zero(kingBB)), position, position.STM());
    };

    int32_t Worker::search(Position& position, Line& PV, uint16_t depth, int32_t alpha, int32_t beta, std::optional<steady_clock::time_point> end) {
        if (end && steady_clock::now() >= *end) return 0;

        if (depth <= 0) {
            return quiesce(position, alpha, beta, end);
        }

        ScoredList move_list = generate_scored_moves(position);
        int32_t best = -INF;
        Line candidate = {};

        for (size_t i = 0; i < move_list.count(); i++) { 
            pick_move(move_list, i);
            Move move = move_list.list(i).move;

            accumulator = eval::update(position, move, accumulator);
            accum_history.emplace_back(accumulator);
            position.make_move(move);

            if (!is_legal(position)) {
                position.unmake_move();
                accum_history.pop_back();
                accumulator = accum_history.back();

                continue;
            }

            nodes++;

            int32_t score = -search(position, candidate, depth - 1, -beta, -alpha, end);

            position.unmake_move();
            accum_history.pop_back();
            accumulator = accum_history.back();
            
            if (end && steady_clock::now() >= *end) return 0;
            
            if (score > best) {
                best = score;
            }

            if (score > alpha) {    
                alpha = score;
                PV.update_line(move, candidate);

                if (score >= beta) {
                    break;
                }
            }
        };  

        return best;
    }

    int32_t Worker::quiesce(Position& position, int32_t alpha, int32_t beta, std::optional<steady_clock::time_point> end) {
        if (end && steady_clock::now() >= *end) return 0;
        
        int32_t eval = eval::evaluate(accumulator);

        int32_t best = eval;
        if (best >= alpha) {
            alpha = best;

            if (best > beta) {
                return best;
            }
        };

        ScoredList captures_list = generate_scored_captures(position);

        for (size_t i = 0; i < captures_list.count(); i++) {
            pick_move(captures_list, i);
            Move move = captures_list.list(i).move;

            accumulator = eval::update(position, move, accumulator);
            accum_history.emplace_back(accumulator);
            position.make_move(move);

            if (!is_legal(position)) {
                position.unmake_move();
                accum_history.pop_back();
                accumulator = accum_history.back();

                continue;
            }

            nodes++;

            int32_t score = -quiesce(position, -beta, -alpha, end);

            position.unmake_move();
            accum_history.pop_back();
            accumulator = accum_history.back();
            
            if (end && steady_clock::now() >= *end) return 0;

            if (score > best) {
                best = score;
            }

            if (score >= alpha) {
                alpha = score;

                if (score > beta) {
                    break;
                }
            }
        }

        return best;
    }

    std::pair<int32_t, Move> Worker::run(const Parameters& params) {
        int32_t result = -1;

        Line PV = {};
        Position position = params.position;

        int32_t time = params.time[color_idx(position.STM())];
        int32_t inc = params.inc[color_idx(position.STM())];

        auto start = steady_clock::now();
        auto end = start + milliseconds(time / 20 + inc / 2);

        for (int depth = 1; depth < MAX_SEARCH_PLY; depth++) {
            if (steady_clock::now() >= end) break;
            result = search(position, PV, depth, -INF, INF, end);
        };

        return {result, PV.moves[0]};
    }

    void Worker::bench(int depth) {
        uint64_t total = 0;
        milliseconds elapsed = 0ms;
        for (std::string fen : fens) {
            Line PV = {};
            Position position;

            position.from_FEN(fen);
            accumulator = eval::reset(position);
            accum_history.emplace_back(accumulator);
            nodes = 0;

            auto start = steady_clock::now();
            int32_t _ = search(position, PV, depth, -INF, INF, std::nullopt);
            auto end = steady_clock::now();

            elapsed += duration_cast<milliseconds>(end - start);
            total += nodes;
        }
        
        std::cout << total << " nodes " << 1000 * total / elapsed.count() << " nps" << std::endl;
    }
}