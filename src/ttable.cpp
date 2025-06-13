#include "ttable.h"

namespace episteme::tt {
    TTable::TTable(uint32_t size) {
        const size_t entries = (size * 1024 * 1024) / sizeof(TTEntry);
        ttable.reserve(entries);
    }
}