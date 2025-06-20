#include "search.h"
#include "bench.h"

#include <cassert>

namespace episteme::search {
    using namespace std::chrono;

    void pick_move(ScoredList& scored_list, int start) {
        for (size_t i = start + 1; i < scored_list.count(); i++)    {
            if (scored_list.list(i).score > scored_list.list(start).score) {
                scored_list.swap(start, i);
            }
        }
    }

    bool in_check(const Position& position, Color color) {
        uint64_t kingBB = position.bitboard(piece_type_idx(PieceType::King)) & position.bitboard(color_idx(color) + position.COLOR_OFFSET);
        return is_square_attacked(sq_from_idx(std::countr_zero(kingBB)), position, flip(color));
    };

    template<typename F>
    ScoredList Thread::generate_scored_targets(const Position& position, F generator, const tt::Entry& tt_entry) {
        MoveList move_list;
        generator(move_list, position);
        ScoredList scored_list;

        for (size_t i = 0; i < move_list.count(); i++) {
            scored_list.add(score_move(position, move_list.list(i), tt_entry));
        }

        return scored_list;
    }

    ScoredMove Thread::score_move(const Position& position, const Move& move, const tt::Entry& tt_entry) {
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
            scored_move.score += history.get_butterfly(position.STM(), move).value;
        }

        return scored_move;
    }

    int32_t Thread::search(Position& position, Line& PV, int16_t depth, int16_t ply, int32_t alpha, int32_t beta, SearchLimits limits = {}) {
        if (limits.time_exceeded()) return 0;

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

        ScoredList move_list = generate_scored_moves(position, tt_entry);
        int32_t best = -INF;
        tt::NodeType node_type = tt::NodeType::AllNode;
        uint32_t num_legal = 0;

        for (size_t i = 0; i < move_list.count(); i++) { 
            pick_move(move_list, i);
            Move move = move_list.list(i).move;

            bool is_quiet = position.mailbox(sq_idx(move.to_square())) == Piece::None && move.move_type() != MoveType::EnPassant;

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
                    if (is_quiet) {
                        history.update_butterfly(position.STM(), move, hist::history_bonus(depth));
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
        if (limits.time_exceeded()) return 0;
        
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

        for (size_t i = 0; i < captures_list.count(); i++) {
            pick_move(captures_list, i);
            Move move = captures_list.list(i).move;

            PieceType src = piece_type(position.mailbox(sq_idx(move.from_square())));
            PieceType dst = piece_type(position.mailbox(sq_idx(move.to_square())));
            int src_val = piece_vals[piece_type_idx(src)];
            int dst_val = move.move_type() == MoveType::EnPassant ? piece_vals[piece_type_idx(PieceType::Pawn)] : piece_vals[piece_type_idx(dst)];

            if (dst_val - src_val < 0) continue;

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
            
            if (limits.time_exceeded()) return 0;

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
        }

        return best;
    }

    ThreadReport Thread::run(const Parameters& params, const SearchLimits& limits) {
        Line PV = {};
        int64_t elapsed = 0;
        int64_t nps = 0;

        Position position = params.position;
        accumulator = eval::reset(position);
        accum_history.emplace_back(accumulator);

        auto start = steady_clock::now();
        int32_t score = search(position, PV, params.depth, 0, -INF, INF, limits);

        elapsed = duration_cast<milliseconds>(steady_clock::now() - start).count();
        nps = (elapsed > 0) ? (1000 * nodes) / elapsed : nodes;

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
            int32_t _ = search(position, PV, depth, 0, -INF, INF);
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

        for (int depth = 1; depth <= max_depth; ++depth) {
            Parameters iter_params = params;
            iter_params.depth = depth;

            ThreadReport report = thread.run(iter_params, limits);
            
            if ((target_nodes && limits.node_exceeded(report.nodes)) || (limits.end != time_point<steady_clock>() && limits.time_exceeded())) break;

            last_report = report;

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