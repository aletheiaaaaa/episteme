#include "movepick.h"

namespace episteme {
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
}