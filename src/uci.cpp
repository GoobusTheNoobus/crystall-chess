#include "uci.hpp"
#include "position.hpp"
#include "search.hpp"

#include <thread>
#include <sstream>

namespace Crystall::UCI {

    namespace {

    // Current position object, controlled via the position UCI command
    Position position;

    // We run the search on a different thread so main thread can stay on UCI loop
    std::thread search_thread;

    void stop() {
        Search::stop();

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

        // Movetime parameter has priority over other
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
            try {
                Search::start(position, depth, time_limit);
            } catch (const std::exception& e) {
                std::cout << "Search crashed: " << e.what() << std::endl;
            } catch (...) {
                std::cout << "Search crashed due to unknown causes" << std::endl;
            }
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
