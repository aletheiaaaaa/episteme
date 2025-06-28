#include "search.h"
#include "bench.h"

#include <cassert>

namespace episteme::search {
    using namespace std::chrono;

    void pick_move(ScoredList& scored_list, int start) {
        for (size_t i = start + 1; i < scored_list.count; i++)    {
            if (scored_list.list[i].score > scored_list.list[start].score) {
                scored_list.swap(start, i);
            }
        }
    }

    bool in_check(const Position& position, Color color) {
        uint64_t king_bb = position.piece_bb(PieceType::King, color);

        return is_square_attacked(sq_from_idx(std::countr_zero(king_bb)), position, flip(color));
    };

    template<typename F>
    ScoredList Thread::generate_scored_targets(const Position& position, F generator, const tt::Entry& tt_entry, std::optional<int32_t> ply) {
        MoveList move_list;
        generator(move_list, position);
        ScoredList scored_list;

        for (size_t i = 0; i < move_list.count; i++) {
            scored_list.add(score_move(position, move_list.list[i], tt_entry, ply));
        }

        return scored_list;
    }

    ScoredMove Thread::score_move(const Position& position, const Move& move, const tt::Entry& tt_entry, std::optional<int32_t> ply) {
        ScoredMove scored_move{.move = move};

        if (tt_entry.move.data() == move.data()) {
            scored_move.score = 1000000;
            return scored_move;
        }

        Piece src = position.mailbox(sq_idx(move.from_square()));
        Piece dst = position.mailbox(sq_idx(move.to_square()));

        bool is_capture = dst != Piece::None || move.move_type() == MoveType::EnPassant;

        if (is_capture) {
            int32_t src_val = piece_vals[piece_type_idx(src)];
            int32_t dst_val = move.move_type() == MoveType::EnPassant ? piece_vals[piece_type_idx(PieceType::Pawn)] : piece_vals[piece_type_idx(dst)];

            scored_move.score += dst_val * 10 - src_val + 100000;

        } else {
            if (ply && stack[*ply].killer.data() == move.data()) {
                scored_move.score = 80000;
                return scored_move;
            }

            scored_move.score += history.get_quiet_hist(position.STM(), move).value;
        }

        return scored_move;
    }

    template<bool PV_node>
    int32_t Thread::search(Position& position, Line& PV, int16_t depth, int16_t ply, int32_t alpha, int32_t beta, SearchLimits limits) {
        if (nodes % 2000 == 0 && limits.time_exceeded()) return 0;

        if (position.is_threefold()) return 0;

        if (depth <= 0) {
            return quiesce(position, PV, ply, alpha, beta, limits);
        }

        tt::Entry tt_entry = ttable.probe(position.zobrist());
        if (ply > 0 && (tt_entry.depth >= depth
            && ((tt_entry.node_type == tt::NodeType::PVNode)
                || (tt_entry.node_type == tt::NodeType::AllNode && tt_entry.score <= alpha)
                || (tt_entry.node_type == tt::NodeType::CutNode && tt_entry.score >= beta))
            )
        ) {
            return tt_entry.score;
        }

        constexpr bool is_PV = PV_node;

        int32_t static_eval;
        if (!is_PV && !in_check(position, position.STM())) {
            static_eval = eval::evaluate(accumulator, position.STM());
        }

        if (!is_PV && !in_check(position, position.STM())) {
            if (depth <= 5 && static_eval >= beta + depth * 100) return static_eval;

            if (depth >= 3) {
                const uint64_t no_pawns_or_kings = position.color_bb(position.STM()) & ~position.piece_bb(PieceType::King, position.STM()) & ~position.piece_bb(PieceType::Pawn, position.STM());

                if (no_pawns_or_kings) {
                    Line null{};

                    position.make_null();
                    int32_t score = -search<false>(position, null, depth - 3, ply + 1, -beta, -beta + 1, limits);
                    position.unmake_move();
                    if (score >= beta) {
                        if (std::abs(score) >= MATE - MAX_SEARCH_PLY) return beta;
                        return score;
                    }
                }
            }
        }

        int16_t search_depth = depth - 1;
        bool no_tt_move = tt_entry.hash != position.zobrist() || tt_entry.move.data() == 0x0000;

        if (no_tt_move && depth >= 4) search_depth--;

        ScoredList move_list = generate_scored_moves(position, tt_entry, ply);
        int32_t best = -INF;

        MoveList explored_quiets;
        tt::NodeType node_type = tt::NodeType::AllNode;
        int32_t num_legal = 0;


        for (size_t i = 0; i < move_list.count; i++) { 
            pick_move(move_list, i);
            Move move = move_list.list[i].move;

            bool is_quiet = position.mailbox(sq_idx(move.to_square())) == Piece::None && move.move_type() != MoveType::EnPassant;

            if (best > -MATE + MAX_SEARCH_PLY) {
                if (is_quiet && num_legal >= 6 + 2 * depth * depth) break;
            
                if (!is_PV && is_quiet && !in_check(position, position.STM()) && static_eval + depth * 250 <= alpha) break;
            }

            accumulator = eval::update(position, move, accumulator);
            accum_history.emplace_back(accumulator);
            position.make_move(move);

            if (in_check(position, position.NTM())) {
                position.unmake_move();
                accum_history.pop_back();
                accumulator = accum_history.back();

                continue;
            }

            nodes++;
            num_legal++;

            if (is_quiet) explored_quiets.add(move);

            if (limits.node_exceeded(nodes)) return 0;

            Line candidate = {};
            int32_t score = 0;
            int search_depth = depth - 1;

            if (num_legal >= 4 && depth >= 3) {
                int16_t reduction = 1;
                int16_t reduced = std::min(std::max(search_depth - reduction, 1), search_depth);

                score = -search<false>(position, candidate, reduced, ply + 1, -alpha - 1, -alpha, limits);
                if (score > alpha && reduced < depth - 1) {
                    score = -search<false>(position, candidate, search_depth, ply + 1, -alpha - 1, -alpha, limits);
                }
            } else if (!is_PV || num_legal > 1) {
                score = -search<false>(position, candidate, search_depth, ply + 1, -alpha - 1, -alpha, limits);
            }

            if (is_PV && (num_legal == 1 || score > alpha)) {
                score = -search<true>(position, candidate, search_depth, ply + 1, -beta, -alpha, limits);
            }

            position.unmake_move();
            accum_history.pop_back();
            accumulator = accum_history.back();
            
            if (nodes % 2000 == 0 && limits.time_exceeded()) return 0;
            
            if (score > best) {
                best = score;
            }

            if (score > alpha) {    
                alpha = score;
                node_type = tt::NodeType::PVNode;

                PV.update_line(move, candidate);

                if (score >= beta) {
                    if (is_quiet) {
                        stack[ply] = {
                            .ply = ply,
                            .killer = move,
                        };

                        history.update_quiet_hist(position.STM(), move, hist::history_bonus(depth));
                        for (size_t j = 0; j < explored_quiets.count; j++) {
                            if (explored_quiets.list[j].data() == move.data()) continue;
                            history.update_quiet_hist(position.STM(), explored_quiets.list[j], hist::history_malus(depth));
                        }
                    }

                    node_type = tt::NodeType::CutNode;
                    break;
                }
            }
        };

        if (num_legal == 0) return in_check(position, position.STM()) ? (-MATE + ply) : 0;

        ttable.add({
            .hash = position.zobrist(),
            .move = PV.moves[0],
            .score = best,
            .depth = static_cast<uint8_t>(depth),
            .node_type = node_type
        });

        return best;
    }

    int32_t Thread::quiesce(Position& position, Line& PV, int16_t ply, int32_t alpha, int32_t beta, SearchLimits limits) {
        if (nodes % 2000 == 0 && limits.time_exceeded()) return 0;
        
        tt::Entry tt_entry = ttable.probe(position.zobrist());
        if ((tt_entry.node_type == tt::NodeType::PVNode)
            || (tt_entry.node_type == tt::NodeType::AllNode && tt_entry.score <= alpha)
            || (tt_entry.node_type == tt::NodeType::CutNode && tt_entry.score >= beta)
        ) {
            return tt_entry.score;
        }

        int32_t eval = eval::evaluate(accumulator, position.STM());

        int32_t best = eval;
        if (best > alpha) {
            alpha = best;

            if (best >= beta) {
                return best;
            }
        };

        ScoredList captures_list = generate_scored_captures(position, tt_entry);
        tt::NodeType node_type = tt::NodeType::AllNode;

        for (size_t i = 0; i < captures_list.count; i++) {
            pick_move(captures_list, i);
            Move move = captures_list.list[i].move;

            accumulator = eval::update(position, move, accumulator);
            accum_history.emplace_back(accumulator);
            position.make_move(move);

            if (in_check(position, position.NTM())) {
                position.unmake_move();
                accum_history.pop_back();
                accumulator = accum_history.back();

                continue;
            }

            nodes++;
            if (limits.node_exceeded(nodes)) return 0;

            Line candidate = {};
            int32_t score = -quiesce(position, candidate, ply + 1, -beta, -alpha, limits);

            position.unmake_move();
            accum_history.pop_back();
            accumulator = accum_history.back();
            
            if (nodes % 2000 == 0 && limits.time_exceeded()) return 0;

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
        }

        ttable.add({
            .hash = position.zobrist(),
            .move = PV.moves[0],
            .score = best,
            .depth = 0,
            .node_type = node_type
        });

        return best;
    }

    ThreadReport Thread::run(int32_t last_score, const Parameters& params, const SearchLimits& limits) {
        Position position = params.position;
        accumulator = eval::reset(position);
        accum_history.emplace_back(accumulator);

        Line PV{};
        int16_t depth = params.depth;
        int32_t delta = DELTA;

        int32_t alpha = (depth == 1) ? -MATE : last_score - delta;
        int32_t beta = (depth == 1) ? MATE : last_score + delta;

        auto start = steady_clock::now();
        int32_t score = search<true>(position, PV, depth, 0, alpha, beta, limits);

        while (score <= alpha || score >= beta) {
            delta *= 2;
            alpha = last_score - delta;
            beta = last_score + delta;
            score = search<true>(position, PV, depth, 0, alpha, beta, limits);
        }

        int64_t elapsed = duration_cast<milliseconds>(steady_clock::now() - start).count();
        int64_t nps = (elapsed > 0) ? (1000 * nodes) / elapsed : nodes;

        ThreadReport report {
            .depth = params.depth,
            .time = elapsed,
            .nodes = nodes,
            .nps = nps,
            .score = score,
            .line = PV
        };

        return report;
    }

    int32_t Thread::eval(const Parameters& params) {
        Position position = params.position;
        accumulator = eval::reset(position);

        return eval::evaluate(accumulator, position.STM());
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
            int32_t _ = search<true>(position, PV, depth, 0, -INF, INF);
            auto end = steady_clock::now();

            elapsed += duration_cast<milliseconds>(end - start);
            total += nodes;
        }

        int64_t nps = elapsed.count() > 0 ? 1000 * total / elapsed.count() : 0;
        std::cout << total << " nodes " << nps << " nps" << std::endl;
    }

    void Instance::run() {
        Position position = params.position;
        int16_t max_depth = params.depth;
        uint64_t target_nodes = params.nodes;
        int32_t time = params.time[color_idx(position.STM())];
        int32_t inc  = params.inc[color_idx(position.STM())];

        SearchLimits limits;
        if (target_nodes) limits.max_nodes = target_nodes;
        if (time)limits.end = steady_clock::now() + milliseconds(time / 20 + inc / 2);

        thread.reset_nodes();

        ThreadReport last_report;
        int32_t last_score = 0;

        for (int depth = 1; depth <= max_depth; ++depth) {
            Parameters iter_params = params;
            iter_params.depth = depth;

            ThreadReport report = thread.run(last_score, iter_params, limits);
            
            if ((target_nodes && limits.node_exceeded(report.nodes)) || (limits.end != time_point<steady_clock>() && limits.time_exceeded())) break;

            last_report = report;
            last_score = report.score;

            bool is_mate = std::abs(report.score) >= MATE - MAX_SEARCH_PLY;
            int32_t display_score = is_mate
                ? ((1 + MATE - std::abs(report.score)) / 2)
                : report.score;

            std::cout << "info depth " << report.depth
                << " time " << report.time
                << " nodes " << report.nodes
                << " nps " << report.nps
                << " score " << (is_mate ? "mate " : "cp ") << display_score
                << " pv ";

            for (size_t i = 0; i < report.line.length; ++i) {
                std::cout << report.line.moves[i].to_string() << " ";
            }
            std::cout << std::endl;
        }
    
        Move best = last_report.line.moves[0];
        std::cout << "bestmove " << best.to_string() << std::endl;
    }

    void Instance::eval(const Parameters& params) {
        std::cout << "info score cp " << thread.eval(params) << std::endl;
    }

    void Instance::bench(int depth) {
        thread.bench(depth);
    }
}