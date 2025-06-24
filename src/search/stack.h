#include "../chess/move.h"

#include <cstdint>

namespace episteme::stack {
    struct Entry {
        int32_t ply = 0;
        Move killer{};
    };

    class Stack {
        public:
            inline void reset() {
                stack.fill(Entry());
            }

            [[nodiscard]] inline Entry &operator[](int idx) {
                return stack[idx];
            }
        private:
            std::array<Entry, 256> stack;
    };
}