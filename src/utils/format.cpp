#include "format.h"

namespace episteme::datagen {
    Format::Format() {
        moves.reserve(256);
    }

    uint32_t Format::write(std::ostream& stream, uint8_t wdl) {
        static constexpr std::array<uint8_t, sizeof(ScoredMove)> NULL_PADDING{};

        initial.wdl = wdl;

        stream.write(reinterpret_cast<const char*>(&initial), sizeof(PackedBoard));
        stream.write(reinterpret_cast<const char*>(moves.data()), sizeof(ScoredMove) * moves.size());
        stream.write(reinterpret_cast<const char*>(NULL_PADDING.data()), sizeof(ScoredMove));

        return moves.size() + 1;
    }
}