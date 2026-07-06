#pragma once

#include "types.hpp"
#include "move.hpp"
#include "tt.hpp"

#include <iostream>
#include <vector>

namespace Crystall {
    namespace UCI {

        inline void info_depth(int depth, int seldepth, int score, u64 nodes_this_iter, u64 elapsed, u64 total_nodes, const std::vector<Move>& pv) {
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

        inline void info_depth(int depth, const Move& currmove, int currmovenumber) {
            std::cout << "info depth " << depth << " currmove " << currmove.to_string() << " currmovenumber " << currmovenumber << std::endl;
        }
        inline void info_string(const std::string& msg) { std::cout << "info string " << msg << std::endl; }

        void loop();
    }
}