#include "search.hpp"
#include "movelist.hpp"

#include <chrono>
#include <iostream>
#include <atomic>

using namespace std::chrono;

namespace Crystall::Search {

    namespace {

        using TimePoint = steady_clock::time_point;

        struct SearchTimer {
            static TimePoint start_time;
            static int max_time_ms;
            static std::atomic_bool stop_flag;

            static bool timeout_search() {
                if (stop_flag) return true;

                // if the max time is 0, the search is infinite
                if (max_time_ms <= 0) return false;

                TimePoint now = steady_clock::now();
                if (duration_cast<milliseconds>(now - start_time).count() > max_time_ms) return true;

                return false;
            }
        };

        int perft(Position& pos, int depth_left) {
            if (depth_left <= 0) return 1;

            int nodes = 0;

            MoveList moves(pos);
            for (int i = 0; i < moves.size(); ++i) {
                Move move = moves[i];

                bool is_legal = pos.attempt_move(move);
                if (!is_legal) continue;

                nodes += perft(pos, depth_left - 1);

                pos.undo_move();
            }

            return nodes;
        }

    }

    void perft_divide(Position& pos, int depth) {
        int total_nodes = 0;

        MoveList moves(pos);

        TimePoint start = steady_clock::now();
        for (int i = 0; i < moves.size(); ++i) {
            Move move = moves[i];

            bool is_legal = pos.attempt_move(move);
            if (!is_legal) continue;

            int nodes = perft(pos, depth - 1);
            std::cout << move.to_string() << ": " << nodes << std::endl;
            total_nodes += nodes;

            pos.undo_move();
        }
        TimePoint end = steady_clock::now();

        u64 duration = duration_cast<milliseconds>(end - start).count();

        std::cout << "\nTotal Nodes: " << total_nodes << std::endl;
        std::cout << "Perft completed in " << duration << "ms" << std::endl;
        std::cout << "NPS: " << (total_nodes / duration * 1000) << std::endl; 

    }

    void start_search(Position& pos, int depth, int movetime) {
        
    }
}
