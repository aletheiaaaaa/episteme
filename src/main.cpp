#include "chess/movegen.h"
#include "chess/perft.h"
#include "search/search.h"
#include "search/bench.h"
#include "uci/uci.h"

#include <cstdint>
#include <array>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <string>

using namespace episteme;

int main(int argc, char *argv[]) {
    zobrist::init();
    search::Config cfg = {};

    if (argc > 1) {
        std::string cmd;
        for (int i = 1; i < argc; ++i) {
            cmd += argv[i];
            if (i < argc - 1) cmd += ' ';
        }
        uci::parse(cmd, cfg);

    } else {
        std::string line;
        while (std::getline(std::cin, line)) {
            uci::parse(line, cfg);    
        }    
    }

    return 0;
}

