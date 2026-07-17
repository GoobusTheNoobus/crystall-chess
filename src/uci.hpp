// Crystall is a hobby UCI chess engine written in C++
// Developed by GoobusTheNoobus

#pragma once

#include "types.hpp"
#include "move.hpp"
#include "tt.hpp"

#include <iostream>
#include <vector>

namespace Crystall {
    namespace UCI {

        void info_depth(int depth, int seldepth, int score, u64 elapsed, u64 total_nodes, const std::vector<Move>& pv);
        void info_depth(int depth, u64 elasped, const Move& currmove, int currmovenumber);
        void info_string(const std::string& msg);

        void loop();
    }
}