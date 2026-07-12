#pragma once

#include "chess/types.hpp"
#include "chess/move/move.hpp"
#include "engine/search/tt.hpp"

#include <iostream>
#include <vector>

namespace Crystall {
    namespace UCI {

        void info_depth(int depth, int seldepth, int score, u64 nodes_this_iter, u64 elapsed, u64 total_nodes, const std::vector<Move>& pv);
        void info_depth(int depth, const Move& currmove, int currmovenumber);
        void info_string(const std::string& msg);

        void loop();
    }
}