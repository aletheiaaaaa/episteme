#include "chess/movegen.h"
#include "chess/perft.h"
#include "search.h"
#include "uci.h"

#include <cstdint>
#include <array>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <string>

using namespace episteme;

int main(int argc, char *argv[]) {
    search::Parameters params = {};

    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            parse(argv[i], params);
        }
        return 0;
    }

    std::string line;
    while (std::getline(std::cin, line)) {
        parse(line, params);
    }
    
    return 0;
}