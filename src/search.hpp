#pragma once

#include "types.hpp"
#include "position.hpp"

namespace Crystall::Search {
    constexpr int MaxSearchDepth = 32;
    inline int history_table[ColorNB][SquareNB][SquareNB];

    struct SearchInfo {
        u64 nodes_searched = 0;
        u64 previous_nodes_searched = 0;
        int plies_from_root = 0;
        int seldepth = 0;
    };

    struct RootSearchResult {
        Move move;
        int score = 0;
    };

    // We copy the position during search
    void start(Position pos, int depth, int movetime);
    void stop();
    void clear_history_table();

    void init();
}