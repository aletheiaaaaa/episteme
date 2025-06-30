#include "engine/chess/movegen.h"
#include "engine/chess/perft.h"
#include "engine/search/search.h"
#include "engine/search/bench.h"
#include "engine/uci/uci.h"

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
    search::Instance instance(cfg);

    if (argc > 1) {
        std::string cmd;
        for (int i = 1; i < argc; ++i) {
            cmd += argv[i];
            if (i < argc - 1) cmd += ' ';
        }
        uci::parse(cmd, cfg, instance);

    } else {
        std::string line;
        while (std::getline(std::cin, line)) {
            uci::parse(line, cfg, instance);    
        }    
    }

    return 0;
}

