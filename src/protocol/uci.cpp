#include "protocol/uci.hpp"
#include "chess/board/position.hpp"
#include "engine/engine.hpp"
#include "engine/search/search.hpp"
#include "engine/search/history.hpp"

#include <thread>
#include <sstream>

namespace Crystall::UCI {

    void info_depth(int depth, int seldepth, int score, u64 nodes_this_iter, u64 elapsed, u64 total_nodes, const std::vector<Move>& pv) {
        std::cout << "info depth " << depth <<
                        " seldepth " << seldepth <<
                        " score " << score_string(score) << 
                        " nodes " << nodes_this_iter << 
                        " nps " << total_nodes * 1000 / std::max<u64>(1ULL, elapsed) << 
                        " hashfull " << TranspositionTable::hashfull() <<
                        " time " << std::max<u64>(1ULL, elapsed) <<
                        " pv ";
        
        for (const Move& m: pv) 
            std::cout << m.to_string() << ' ';
        
        std::cout << std::endl;
    }

    void info_depth(int depth, const Move& currmove, int currmovenumber) {
        std::cout << "info depth " << depth << " currmove " << currmove.to_string() << " currmovenumber " << currmovenumber << std::endl;
    }
    void info_string(const std::string& msg) { std::cout << "info string " << msg << std::endl; }

    namespace {

    Position position;
    std::thread search_thread;

    void stop() {
        Engine::stop();

        if (search_thread.joinable()) {
            search_thread.join();
        }
    }

    void handle_uci() {
        std::cout << "id name Crystall \n" <<
                     "id author GoobusTheNoobus\n" <<
                     "uciok" <<
                     std::endl;    
    }

    void handle_go(std::istringstream& iss) {
        stop();

        if (search_thread.joinable()) {
            search_thread.join();
        }

        std::string token;

        int depth = 0;
        int movetime = 0;

        int winc = 0;
        int binc = 0;
        int wtime = 0;
        int btime = 0;
        
        while (iss >> token) {
            if (token == "depth") {
                iss >> depth;
            } else if (token == "movetime") {
                iss >> movetime;
            } else if (token == "wtime") {
                iss >> wtime;
            } else if (token == "btime") {
                iss >> btime;
            } else if (token == "winc") {
                iss >> winc;
            } else if (token == "binc") {
                iss >> binc;
            }
        }

        int time_limit = 0;

        if (movetime > 0) {
            time_limit = movetime;
        }

        else if (wtime > 0 || btime > 0) {
            int our_time = position.get_side_to_move() == White ? wtime : btime;
            int our_inc  = position.get_side_to_move() == White ? winc : binc;

            time_limit = std::min(our_time / 20 + our_inc / 2, our_time);
        }

        if (depth < 1 || depth > Search::MaxSearchDepth) depth = Search::MaxSearchDepth;
        
        search_thread = std::thread([depth, time_limit]() {
            Engine::start(position, depth, time_limit);
        });
    }

    void handle_position(std::istringstream& iss) {
        stop();

        std::string token;
        iss >> token;

        if (token == "startpos") {
            position.parse_fen(StartingPositionFen);

            iss >> token;
        } else if (token == "fen") {
            std::string fen;

            while ((iss >> token) && token != "moves") {
                fen += token + " ";
            }

            position.parse_fen(fen);
        }

        if (token == "moves") {
            while (iss >> token) {
                position.make_move(token);
            }
        }
    }
    void handle_isready() {
        std::cout << "readyok" << std::endl;
    }
    void ucinewgame() {
        TranspositionTable::clear();
        Search::History::clear();
    }

    void dispatch(const std::string& cmd, std::istringstream& iss) {
        if (cmd == "uci") {
            handle_uci();
        }
        else if (cmd == "isready") {
            handle_isready();
        }
        else if (cmd == "go") {
            handle_go(iss);
        }
        else if (cmd == "position") {
            handle_position(iss);
        }
        else if (cmd == "ucinewgame") {
            ucinewgame();
        }
        else if (cmd == "d") {
            std::cout << position << std::endl;
        }
        else {
            info_string("invalid command");
        }
    }
    }

    void loop() {

        while (true) {
            std::string command;
            if (!std::getline(std::cin, command)) {
                stop();
                break;
            }
            std::istringstream iss(command);
            std::string token;

            if (command.find_first_not_of(" ") == std::string::npos) continue;

            iss >> token;
            if (token == "quit") {
                stop();
                break;
            } else if (token == "stop") {
                stop();
                continue;
            }

            dispatch(token, iss);
        }

        stop();
    }
}
