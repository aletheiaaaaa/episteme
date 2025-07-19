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

    template<typename F>
    ScoredList Worker::generate_scored_targets(const Position& position, F generator, const tt::Entry& tt_entry, std::optional<int32_t> ply) {
        MoveList move_list;
        generator(move_list, position);
        ScoredList scored_list;

        for (size_t i = 0; i < move_list.count; i++) {
            scored_list.add(score_move(position, move_list.list[i], tt_entry, ply));
        }

        return scored_list;
    }

    ScoredMove Worker::score_move(const Position& position, const Move& move, const tt::Entry& tt_entry, std::optional<int32_t> ply) {
        ScoredMove scored_move{.move = move};

        if (tt_entry.move.data() == move.data()) {
            scored_move.score = 10000000;
            return scored_move;
        }

        Piece src = position.mailbox(move.from_square());
        Piece dst = position.mailbox(move.to_square());

        bool is_capture = dst != Piece::None || move.move_type() == MoveType::EnPassant;

        if (is_capture) {
            int32_t src_val = piece_vals[piece_type_idx(src)];
            int32_t dst_val = move.move_type() == MoveType::EnPassant ? piece_vals[piece_type_idx(PieceType::Pawn)] : piece_vals[piece_type_idx(dst)];

            scored_move.score += dst_val * 10 - src_val;
            if (eval::SEE(position, move, 0)) scored_move.score += 1000000;
        } else {
            if (stack[*ply].killer.data() == move.data()) {
                scored_move.score = 800000;
                return scored_move;
            }

            scored_move.score += history.get_quiet_hist(position.STM(), move);
            scored_move.score += history.get_cont_hist(stack, src, move, *ply);
        }

        return scored_move;
    }

    std::array<std::array<int16_t, 64>, 64> lmr_table{};
    void init_lmr_table() {
        for (int i = 1; i < 64; i++) {
            for (int j = 1; j < 64; j++) {
                lmr_table[i][j] = 0.5 + std::log(i) * std::log(j) / 3.0;
            }
        }
    }

    template<bool PV_node>
    int32_t Worker::search(Position& position, Line& PV, int16_t depth, int16_t ply, int32_t alpha, int32_t beta, SearchLimits limits) {
        if (nodes % 2000 == 0 && limits.time_exceeded()) {
            should_stop = true;
            return 0;
        };

        if (ply > 0 && position.is_threefold()) return 0;

        if (depth <= 0) {
            return quiesce(position, PV, ply, alpha, beta, limits);
        }

        stack[ply].move = Move();
        stack[ply].piece = Piece::None;

        tt::Entry tt_entry{};
        if (!stack[ply].excluded.data()) {
            tt_entry = ttable.probe(position.zobrist());
            if (ply > 0 && (tt_entry.depth >= depth
                && ((tt_entry.node_type == tt::NodeType::PVNode)
                    || (tt_entry.node_type == tt::NodeType::AllNode && tt_entry.score <= alpha)
                    || (tt_entry.node_type == tt::NodeType::CutNode && tt_entry.score >= beta))
                )
            ) {
                return tt_entry.score;
            }    
        }

        constexpr bool is_PV = PV_node;

        int32_t static_eval = -INF;
        if (!in_check(position, position.STM())) {
            static_eval = eval::evaluate(accumulator, position.STM());
            stack[ply].eval = static_eval;
        } 

        bool improving = false;

        if (!in_check(position, position.STM())) {
            if (ply > 1 && stack[ply - 2].eval != -INF) {
                improving = static_eval > stack[ply - 2].eval;
            }
        }

        if (!stack[ply].excluded.data() && !in_check(position, position.STM())) {
            if (!is_PV && depth <= 5 && static_eval >= beta + std::max(depth - improving, 0) * 100) return static_eval;

            if (!is_PV && depth >= 3) {
                const uint64_t no_pawns_or_kings = position.color_bb(position.STM()) & ~position.piece_bb(PieceType::King, position.STM()) & ~position.piece_bb(PieceType::Pawn, position.STM());

                if (no_pawns_or_kings) {
                    Line null{};
                    int16_t reduction = 3 + improving;

                    stack[ply].move = Move();
                    stack[ply].piece = Piece::None;

                    position.make_null();
                    int32_t score = -search<false>(position, null, depth - reduction, ply + 1, -beta, -beta + 1, limits);
                    position.unmake_move();

                    if (should_stop) return 0;

                    if (score >= beta) {
                        if (std::abs(score) >= MATE - MAX_SEARCH_PLY) return beta;
                        return score;
                    }
                }
            }
        }

        ScoredList move_list = generate_scored_moves(position, tt_entry, ply);
        int32_t best = -INF;

        MoveList explored_quiets;
        tt::NodeType node_type = tt::NodeType::AllNode;
        int32_t num_legal = 0;

        for (size_t i = 0; i < move_list.count; i++) { 
            pick_move(move_list, i);
            Move move = move_list.list[i].move;
            Piece piece = position.mailbox(move.from_square());

            bool is_quiet = position.mailbox(move.to_square()) == Piece::None && move.move_type() != MoveType::EnPassant;

            if (ply > 0 && best > -MATE + MAX_SEARCH_PLY) {
                const int32_t lmp_threshold = 6 + 2 * depth * depth;
                if (is_quiet && num_legal >= lmp_threshold) break;

                const int32_t fp_margin = depth * 250;
                if (!is_PV && is_quiet && !in_check(position, position.STM()) && static_eval + fp_margin <= alpha) break;

                const int32_t see_threshold = (is_quiet) ? -60 * depth : -30 * depth * depth;
                if (!is_PV && !eval::SEE(position, move, see_threshold)) continue;
            }

            if (move.data() == stack[ply].excluded.data()) continue;

            int16_t extension = 0;
            if (ply > 0 && depth >= 8 && move.data() == tt_entry.move.data() && !stack[ply].excluded.data() && tt_entry.depth >= depth - 3 && tt_entry.node_type != tt::NodeType::AllNode) {
                const int32_t new_beta = std::max(-INF + 1, tt_entry.score - depth * 2);
                const int16_t new_depth = (depth - 1) / 2;

                stack[ply].excluded = move;
                int32_t score = search<false>(position, PV, new_depth, ply, new_beta - 1, new_beta, limits);
                stack[ply].excluded = Move();

                if (should_stop) return 0;

                if (score < new_beta) extension = 1;
                else if (new_beta >= beta && std::abs(score) < MATE - MAX_SEARCH_PLY) return new_beta;
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
            stack[ply].move = move;
            stack[ply].piece = piece;

            if (is_quiet) explored_quiets.add(move);

            if (limits.node_exceeded(nodes)) {
                should_stop = true;
                position.unmake_move();
                return 0;
            };

            Line candidate = {};
            int32_t score = 0;
            int16_t new_depth = depth - 1 + extension;

            if (num_legal >= 4 && depth >= 3) {
                int16_t reduction = lmr_table[depth][num_legal] + !improving;
                int16_t reduced = std::min(std::max(new_depth - reduction, 1), static_cast<int>(new_depth));

                score = -search<false>(position, candidate, reduced, ply + 1, -alpha - 1, -alpha, limits);
                if (score > alpha && reduced < depth - 1) {
                    score = -search<false>(position, candidate, new_depth, ply + 1, -alpha - 1, -alpha, limits);
                }
            } else if (!is_PV || num_legal > 1) {
                score = -search<false>(position, candidate, new_depth, ply + 1, -alpha - 1, -alpha, limits);
            }

            if (is_PV && (num_legal == 1 || score > alpha)) {
                score = -search<true>(position, candidate, new_depth, ply + 1, -beta, -alpha, limits);
            }

            position.unmake_move();
            accum_history.pop_back();
            accumulator = accum_history.back();
            
            if (should_stop) return 0;
            
            if (score > best) {
                best = score;
            }

            if (score > alpha) {    
                alpha = score;
                node_type = tt::NodeType::PVNode;

                PV.update_line(move, candidate);

                if (score >= beta) {
                    if (is_quiet) {
                        stack[ply].killer = move;

                        int16_t bonus = hist::history_bonus(depth);
                        history.update_quiet_hist(position.STM(), move, bonus);
                        history.update_cont_hist(stack, piece, move, bonus, ply);

                        for (size_t j = 0; j < explored_quiets.count; j++) {
                            if (explored_quiets.list[j].data() == move.data()) continue;

                            int16_t malus = hist::history_malus(depth);
                            history.update_quiet_hist(position.STM(), explored_quiets.list[j], malus);
                            history.update_cont_hist(stack, piece, explored_quiets.list[j], malus, ply);
                        }
                    }

                    node_type = tt::NodeType::CutNode;
                    break;
                }
            }
        };

        if (num_legal == 0) return in_check(position, position.STM()) ? (-MATE + ply) : 0;

        if (!stack[ply].excluded.data()) {
            ttable.add({
                .hash = position.zobrist(),
                .move = PV.moves[0],
                .score = best,
                .depth = static_cast<uint8_t>(depth),
                .node_type = node_type
            });    
        }

        return best;
    }

    int32_t Worker::quiesce(Position& position, Line& PV, int16_t ply, int32_t alpha, int32_t beta, SearchLimits limits) {
        if (nodes % 2000 == 0 && limits.time_exceeded()) {
            should_stop = true;
            return 0;
        };
        
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

            if (!eval::SEE(position, move, 0)) continue;

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

            if (limits.node_exceeded(nodes)) {
                should_stop = true;
                position.unmake_move();
                return 0;
            };

            Line candidate = {};
            int32_t score = -quiesce(position, candidate, ply + 1, -beta, -alpha, limits);

            position.unmake_move();
            accum_history.pop_back();
            accumulator = accum_history.back();
            
            if (should_stop) return 0;

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

    Report Worker::run(int32_t last_score, const Parameters& params, Position& position, const SearchLimits& limits, bool is_absolute) {
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

        score = (is_absolute) ? score * (!color_idx(position.STM()) ? 1 : -1) : score;

        Report report {
            .depth = params.depth,
            .time = elapsed,
            .nodes = nodes,
            .nps = nps,
            .score = score,
            .line = PV
        };

        return report;
    }

    int32_t Worker::eval(Position& position) {
        accumulator = eval::reset(position);

        return eval::evaluate(accumulator, position.STM());
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
            (void)search<true>(position, PV, depth, 0, -INF, INF);
            auto end = steady_clock::now();

            elapsed += duration_cast<milliseconds>(end - start);
            total += nodes;
        }

        int64_t nps = elapsed.count() > 0 ? 1000 * total / elapsed.count() : 0;
        std::cout << total << " nodes " << nps << " nps" << std::endl;
    }

    void Engine::run(Position& position) {
        int16_t max_depth = params.depth;
        uint64_t target_nodes = params.nodes;
        int32_t time = params.time[color_idx(position.STM())];
        int32_t inc  = params.inc[color_idx(position.STM())];

        SearchLimits limits;
        if (target_nodes) limits.max_nodes = target_nodes;
        if (time)limits.end = steady_clock::now() + milliseconds(time / 20 + inc / 2);

        worker.reset_nodes();

        Report last_report;
        int32_t last_score = 0;

        for (int depth = 1; depth <= max_depth; depth++) {
            Parameters iter_params = params;
            iter_params.depth = depth;

            Report report = worker.run(last_score, iter_params, position, limits, false);
            if (worker.stopped()) break;

            last_report = report;
            last_score = report.score;

            bool is_mate = std::abs(report.score) >= MATE - MAX_SEARCH_PLY;
            int32_t display_score = is_mate ? ((1 + MATE - std::abs(report.score)) / 2) * ((report.score > 0) ? 1 : -1) : report.score;

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

    ScoredMove Engine::datagen_search(Position& position) {
        uint64_t hard_nodes = params.nodes;
        uint64_t soft_nodes = params.soft_nodes;

        SearchLimits limits{};
        limits.max_nodes = hard_nodes;

        worker.reset_nodes();

        Report last_report;
        int32_t last_score = 0;

        for (int depth = 1; depth <= 10; depth++) {
            Parameters iter_params = params;
            iter_params.depth = depth;

            Report report = worker.run(last_score, iter_params, position, limits, true);
            if (worker.stopped()) break;

            last_report = report;
            last_score = report.score;

            if (worker.node_count() > soft_nodes) break;
        }

        ScoredMove best{
            .move = last_report.line.moves[0],
            .score = last_report.score
        };

        return best;
    }

    void Engine::eval(Position& position) {
        std::cout << "info score cp " << worker.eval(position) << std::endl;
    }

    void Engine::bench(int depth) {
        worker.bench(depth);
    }
}