#include "search.h"
#include "bench.h"

namespace episteme::search {
    using namespace std::chrono;

    void pick_move(ScoredList& scored_list, int start) {
        const ScoredMove& start_move = scored_list.list(start);
        for (size_t i = start + 1; i <  scored_list.count(); i++)    {
            if (scored_list.list(i).mvv_lva > start_move.mvv_lva) {
                scored_list.swap(start, i);
            }
        }
    }

    template<typename F>
    ScoredList generate_scored_targets(const Position& position, F generator, bool include_quiets, const std::optional<tt::TTEntry>& tt_entry) {
        MoveList move_list;
        generator(move_list, position);
        ScoredList scored_list;

        for (size_t i = 0; i < move_list.count(); i++) {
            Move move = move_list.list(i);
            bool from_tt = tt_entry && include_quiets && tt_entry->move.data() == move.data();

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
                .mvv_lva = from_tt ? 1000 : mvv_lva
            });
        }

        return scored_list;
    }

    bool is_legal(const Position& position) {
        uint64_t kingBB = position.bitboard(piece_type_idx(PieceType::King)) & position.bitboard(color_idx(position.NTM()) + position.COLOR_OFFSET);
        return !is_square_attacked(sq_from_idx(std::countr_zero(kingBB)), position, position.STM());
    };

    int32_t Thread::search(Position& position, Line& PV, int16_t depth, int16_t ply, int32_t alpha, int32_t beta, SearchLimits limits = {}) {
        if (limits.time_exceeded()) return 0;

        if (depth <= 0) {
            return quiesce(position, ply + 1, alpha, beta, limits);
        }

        tt::TTEntry tt_entry = ttable.probe(position.zobrist());
        if (ply > 0 && (tt_entry.depth >= depth
            && ((tt_entry.node_type == tt::NodeType::PVNode)
                || (tt_entry.node_type == tt::NodeType::AllNode && tt_entry.score <= alpha)
                || (tt_entry.node_type == tt::NodeType::CutNode && tt_entry.score >= beta))
            )
        ) {
            return tt_entry.score;
        }

        ScoredList move_list = generate_scored_moves(position, tt_entry);
        int32_t best = -INF;
        tt::NodeType node_type = tt::NodeType::AllNode;

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
            if (limits.node_exceeded(nodes)) return 0;

            Line candidate = {};
            int32_t score = -search(position, candidate, depth - 1, ply + 1, -beta, -alpha, limits);

            position.unmake_move();
            accum_history.pop_back();
            accumulator = accum_history.back();
            
            if (limits.time_exceeded()) return 0;
            
            if (score > best) {
                best = score;
            }

            if (score > alpha) {    
                alpha = score;
                node_type = tt::NodeType::PVNode;

                PV.update_line(move, candidate);

                if (score >= beta) {
                    node_type = tt::NodeType::CutNode;
                    break;
                }
            }
        };

        ttable.add({
            .hash = position.zobrist(),
            .move = PV.moves[0],
            .score = best,
            .depth = static_cast<uint8_t>(depth),
            .node_type = node_type
        });

        return best;
    }

    int32_t Thread::quiesce(Position& position, int16_t ply, int32_t alpha, int32_t beta, SearchLimits limits) {
        if (limits.time_exceeded()) return 0;
        
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

            PieceType src = piece_type(position.mailbox(sq_idx(move.from_square())));
            PieceType dst = piece_type(position.mailbox(sq_idx(move.to_square())));
            int src_val = piece_vals[piece_type_idx(src)];
            int dst_val = move.move_type() == MoveType::EnPassant ? piece_vals[piece_type_idx(PieceType::Pawn)] : piece_vals[piece_type_idx(dst)];

            if (dst_val - src_val < 0) break;

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
            if (limits.node_exceeded(nodes)) return 0;

            int32_t score = -quiesce(position, ply + 1, -beta, -alpha, limits);

            position.unmake_move();
            accum_history.pop_back();
            accumulator = accum_history.back();
            
            if (limits.time_exceeded()) return 0;

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

    ThreadReport Thread::run(const Parameters& params) {
        int32_t final_score = -1;
        Line PV = {};

        int64_t elapsed = 0;
        int64_t nps = 0;
        int16_t search_depth = 0;

        Position position = params.position;
        accumulator = eval::reset(position);
        accum_history.emplace_back(accumulator);

        int16_t target_depth = params.depth;
        uint64_t target_nodes = params.nodes;
        int32_t time = params.time[color_idx(position.STM())];
        int32_t inc = params.inc[color_idx(position.STM())];

        nodes = 0;

        if (target_nodes) {
            auto start = steady_clock::now();

            SearchLimits limits;
            limits.max_nodes = target_nodes;

            for (int depth = 1; depth < MAX_SEARCH_PLY; depth++) {
                int32_t score = search(position, PV, depth, 0, -INF, INF, limits);
                if (limits.node_exceeded(nodes)) break;

                final_score = score;
                search_depth = depth;
            };

            elapsed = duration_cast<milliseconds>(steady_clock::now() - start).count();
        }

        if (target_depth) {
            auto start = steady_clock::now();

            final_score = search(position, PV, target_depth, 0, -INF, INF);

            search_depth = target_depth;
            elapsed = duration_cast<milliseconds>(steady_clock::now() - start).count();
        }

        if (time) {
            auto start = steady_clock::now();
            auto end = start + milliseconds(time / 20 + inc / 2);

            SearchLimits limits;
            limits.end = end;

            for (int depth = 1; depth < MAX_SEARCH_PLY; depth++) {
                int32_t score = search(position, PV, depth, 0, -INF, INF, limits);
                if (limits.time_exceeded()) break;

                final_score = score;
                search_depth = depth;
            };  

            elapsed = duration_cast<milliseconds>(steady_clock::now() - start).count();
        }

        nps = (elapsed > 0) ? (1000 * nodes) / elapsed : nodes;

        ThreadReport report {
            .depth = search_depth,
            .time = elapsed,
            .nodes = nodes,
            .nps = nps,
            .score = final_score,
            .line = PV
        };

        return report;
    }

    void Thread::bench(int depth) {
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
            int32_t _ = search(position, PV, depth, 0, -INF, INF);
            auto end = steady_clock::now();

            elapsed += duration_cast<milliseconds>(end - start);
            total += nodes;
        }

        int64_t nps = elapsed.count() > 0 ? 1000 * total / elapsed.count() : 0;
        std::cout << total << " nodes " << nps << " nps" << std::endl;    }

    void Instance::run() {
        ThreadReport variation = thread.run(params);

        std::cout << "info depth " << variation.depth 
            << " time " << variation.time 
            << " nodes " << variation.nodes 
            << " nps " << variation.nps
            << " score cp " << variation.score
            << " pv ";

        for (size_t i = 0; i < variation.line.length; i++) {
            std::cout << variation.line.moves[i].to_string() << " ";
        }
        std::cout << std::endl;

        Move best = variation.line.moves[0];
        std::cout << "bestmove " << best.to_string() << std::endl;
    }

    void Instance::bench(int depth) {
        thread.bench(depth);
    }
}