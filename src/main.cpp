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

int main(int argc, char *argv[]) {
    search::Parameters params = {};

    if (argc > 1) {
        std::string cmd;
        for (int i = 1; i < argc; ++i) {
            cmd += argv[i];
            if (i < argc - 1) cmd += ' ';
        }

        if (cmd == "bench") {
            search::Worker thread;
            thread.bench();
        } else {
            parse(cmd, params);
        }
    } else {
        std::string line;
        while (std::getline(std::cin, line)) {
            if (line == "bench") {
                search::Worker thread;
                thread.bench();
            } else {
                parse(line, params);
            }
        }    
    }

    return 0;
}

