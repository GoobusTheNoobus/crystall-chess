#pragma once

#include "types.hpp"
#include "move.hpp"

#include <iostream>

namespace Crystall {
    namespace UCI {

        inline void info_depth(int depth, int score, u64 nodes_this_iter, u64 elapsed, u64 total_nodes, Move pv) {
            std::cout << "info depth " << depth <<
                             " score " << score_string(score) << 
                             " nodes " << nodes_this_iter << 
                             " nps " << total_nodes * 1000 / std::max(1ULL, elapsed) << 
                             " time " << elapsed <<
                             " pv " << pv.to_string() << std::endl;;
        }   
        inline void info_string(const std::string& msg) { std::cout << "info string " << msg << std::endl; }

        void loop();
    }
}