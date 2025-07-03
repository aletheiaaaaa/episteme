#include "format.h"

namespace episteme::datagen {
    Format::Format() {
        moves.reserve(256);
    }

    void Format::push(Move move, int32_t score) {
        static constexpr std::array<uint16_t, 4> move_types = {0x0000, 0x8000, 0x4000, 0xC000};

        uint16_t viri_move = 0;
        viri_move |= (move.from_idx() | (move.to_idx() << 6) | (move.promo_idx() << 12) | move_types[move.type_idx()]);

        moves.push_back({
            .move = viri_move, .score = static_cast<int16_t>(score)}
        );
    }

    size_t Format::write(std::ostream& stream, uint8_t wdl) {
        static constexpr std::array<uint8_t, sizeof(ScoredMove)> NULL_TERMINATOR{};

        initial.wdl = wdl;

        stream.write(reinterpret_cast<const char*>(&initial), sizeof(PackedBoard));
        stream.write(reinterpret_cast<const char*>(moves.data()), sizeof(ScoredMove) * moves.size());
        stream.write(reinterpret_cast<const char*>(NULL_TERMINATOR.data()), sizeof(ScoredMove));

        return moves.size() + 1;
    }
}