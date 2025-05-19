#include "uci.h"

namespace episteme {
    int parse(const std::string& cmd) {
        std::string keyword = cmd.substr(0, cmd.find(' '));

        if (keyword == "uci") uci();
        else if (keyword == "isready") isReady();
        else if (keyword == "position") position(cmd.substr(cmd.find(" ")+1));
        else if (keyword == "go") go(cmd.substr(cmd.find(' ')));
        else if (keyword == "ucinewgame") uciNewGame();
        else if (keyword == "quit") quit();
        else std::cout << "invalid command";

        return 0;
    }

    auto uci() {
        std::cout << "id name Valhalla \nid author Carbon";
    }

    auto isReady() {
        std::cout << "readyok\n";
    }

    auto position(const std::string& params) {
        std::istringstream iss(params);
        std::string token;

        std::string fen;
        int i = 0;
        for (int i = 0; i < 6 && iss >> token; ++i) {
            if (!fen.empty()) fen += " ";
            fen += token;
        }

        Position position;
        position.fromFEN(fen);

        if (iss >> token) {
            if (token == "move") {
                while (iss >> token) {
                    position.makeMove(fromUCI(position, token));
                }
            } else {
                std::cout << "invalid command";
            }
        }
    }
}