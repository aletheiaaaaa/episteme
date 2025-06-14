#include "uci.h"

namespace episteme::uci {

    auto uci() {
        std::cout << "id name Episteme \nid author aletheia\n";
        std::cout << "option name Hash type spin default 32 min 1 max 128\n";
        std::cout << "option name Threads type spin default 1 min 1 max 1\n";
        std::cout << "uciok\n";
    }

    // auto setoption(const std::string& args, search::Parameters& params) {

    // }

    auto isready() {
        std::cout << "readyok\n";
    }

    auto position(const std::string& args, search::Parameters& params) {
        Position position;
        std::istringstream iss(args);
        std::string token;

        iss >> token;

        if (token == "startpos") {
            position.from_start_pos();

        } else if (token == "fen") {    
            std::string fen;
            for (int i = 0; i < 6 && iss >> token; ++i) {
                if (!fen.empty()) fen += " ";
                fen += token;
            }
            position.from_FEN(fen);

        } else {
            std::cout << "invalid command\n";
        }

        if (iss >> token && token == "moves") {
            while (iss >> token) {
                position.make_move(from_UCI(position, token));
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
            else {
                std::cout << "invalid command\n"; 
                break;
            }
        }

        search::Thread thread;
        std::cout << "bestmove " << thread.run(params).second.moves[0].to_string() << std::endl;
    }

    auto ucinewgame(search::Parameters& params) {
        params = {};
    }
    
    auto bench(const std::string& args) {
        int depth = (args.empty()) ? 5 : std::stoi(args);
    
        search::Thread thread;
        thread.bench(depth);
    }

    auto perft(const std::string& args, search::Parameters& params) {
        int depth = (args.empty()) ? 6 : std::stoi(args);
        Position& position = params.position;

        time_perft(position, depth);
    }

    int parse(const std::string& cmd, search::Parameters& params) {
        std::string keyword = cmd.substr(0, cmd.find(' '));

        if (keyword == "uci") uci();
        // else if (keyword == "setoption") setoption(cmd.substr(cmd.find(" ")+1), params);
        else if (keyword == "setoption") return 0;
        else if (keyword == "isready") isready();
        else if (keyword == "position") position(cmd.substr(cmd.find(" ")+1), params);
        else if (keyword == "go") go(cmd.substr(cmd.find(" ")+1), params);
        else if (keyword == "ucinewgame") ucinewgame(params);
        else if (keyword == "quit") std::exit(0);

        else if (keyword == "bench") {
            size_t space = cmd.find(' ');
            std::string arg = (space != std::string::npos) ? cmd.substr(space+1) : "";
            bench(arg);
        }
        else if (keyword == "perft") {
            size_t space = cmd.find(' ');
            std::string arg = (space != std::string::npos) ? cmd.substr(space+1) : "";
            perft(arg, params);
        }

        else std::cout << "invalid command\n";

        return 0;
    }
}