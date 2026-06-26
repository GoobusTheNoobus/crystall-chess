#pragma once

#include "types.hpp"
#include "position.hpp"

namespace Crystall::Search {
    constexpr int MaxSearchDepth = 100;

    struct SearchInfo {
        u64 nodes_searched = 0;
        u64 previous_nodes_searched = 0;
        int plies_from_root = 0;
        int seldepth = 0;
    };

    struct RootSearchResult {
        int score = 0;
        Move move;
    };

    void start_search(Position& pos, int depth, int movetime);
    void stop_search();

    int qsearch_node(SearchInfo& info, Position& pos, int depth, int alpha, int beta);
    int search_node(SearchInfo& info, Position& pos, int depth, int alpha, int beta);
    RootSearchResult search_root(SearchInfo& info, Position& pos, int depth);

    void perft_divide(Position& pos, int depth);
    int perft(Position& pos, int depth_left);
}