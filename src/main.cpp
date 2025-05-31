#include "chess/movegen.h"
#include "chess/perft.h"
#include "search.h"
#include "uci.h"
#include "bench.h"

#include <cstdint>
#include <array>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <string>

using namespace episteme;

// int main(int argc, char *argv[]) {
//     search::Parameters params = {};
//     std::string line;

//     while (std::getline(std::cin, line)) {
//         if (line == "bench") {
//             search::Worker thread;
//             thread.bench();
//         } else {
//             parse(line, params);
//         }
//     }
//     return 0;
// }

int main() {
    search::Worker thread;
    thread.bench();
}
