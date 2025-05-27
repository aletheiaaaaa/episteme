#include "uci.h"

namespace episteme {

    auto uci() {
        std::cout << "id name Episteme \nid author aletheia\n";
        std::cout << "option name Hash type spin default 1 min 1 max 1\n";
        std::cout << "option name Threads type 1 spin default 1 min 1 max 1\n";
        std::cout << "uciok\n";
    }

    auto isReady() {
        std::cout << "readyok\n";
    }

    auto position(const std::string& args, search::Parameters& params) {
        Position position;
        std::istringstream iss(args);
        std::string token;

        iss >> token;

        if (token == "startpos") {
            position.fromStartPos();

        } else if (token == "fen") {    
            std::string fen;
            for (int i = 0; i < 6 && iss >> token; ++i) {
                if (!fen.empty()) fen += " ";
                fen += token;
            }
            position.fromFEN(fen);

        } else {
            std::cout << "invalid command\n";
        }

        if (iss >> token && token == "moves") {
            while (iss >> token) {
                position.makeMove(fromUCI(position, token));
            }
        }

        params.position = position;

    }

    auto go(const std::string& args, search::Parameters& params) {
        std::istringstream iss(args);
        std::string token;

        while (iss >> token) {
            if (token == "wtime" && iss >> token) params.time[0] = std::stoi(token);
            else if (token == "btime" && iss >> token) params.time[1] = std::stoi(token);
            else if (token == "winc" && iss >> token) params.inc[0] = std::stoi(token);
            else if (token == "binc" && iss >> token) params.inc[1] = std::stoi(token);
            else std::cout << "invalid command\n"; break;
        }

        search::Worker worker;
        worker.parameters = params;
        std::cout << worker.run().second.toString() << std::endl;
    }

    auto uciNewGame(search::Parameters& params) {
        params = {};
    }

    int parse(const std::string& cmd, search::Parameters& params) {
        std::string keyword = cmd.substr(0, cmd.find(' '));

        if (keyword == "uci") uci();
        else if (keyword == "isready") isReady();
        else if (keyword == "position") position(cmd.substr(cmd.find(" ")+1), params);
        else if (keyword == "go") go(cmd.substr(cmd.find(' ')+1), params);
        else if (keyword == "ucinewgame") uciNewGame(params);
        else if (keyword == "quit") std::exit(0);
        else std::cout << "invalid command\n";

        return 0;
    }
}