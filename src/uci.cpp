#include "uci.h"

namespace episteme {
    int parse(const std::string& cmd) {
        std::string keyword = cmd.substr(0, cmd.find(' '));
        search::Parameters params;

        if (keyword == "uci") uci();
        else if (keyword == "isready") isReady();
        else if (keyword == "position") position(cmd.substr(cmd.find(" ")+1), params);
        else if (keyword == "go") go(cmd.substr(cmd.find(' ')+1), params);
        else if (keyword == "ucinewgame") uciNewGame(params);
        else if (keyword == "quit") return 0;
        else std::cout << "invalid command";

        return 0;
    }

    auto uci() {
        std::cout << "id name Episteme \nid author Carbon";
    }

    auto isReady() {
        std::cout << "readyok\n";
    }

    auto position(const std::string& args, search::Parameters params) {
        std::istringstream iss(args);
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

        params.position = position;
    }

    auto go(const std::string& args, search::Parameters params) {
        std::istringstream iss(args);
        std::string token;

        while (iss >> token) {
            if (token == "wtime") params.time[0] = std::stoi(token);
            else if (token == "btime") params.time[1] = std::stoi(token);
            else if (token == "winc") params.inc[0] = std::stoi(token);
            else if (token == "binc") params.inc[1] = std::stoi(token);
            else std::cout << "invalid command";
        }
    }

    auto uciNewGame(search::Parameters params) {
        params = {};
    }
}