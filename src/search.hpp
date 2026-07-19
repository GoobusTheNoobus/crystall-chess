// Crystall is a hobby UCI chess engine written in C++
// Developed by GoobusTheNoobus

#pragma once

#include "types.hpp"
#include "position.hpp"
#include "timer.hpp"

namespace Crystall::Search {
    constexpr int MaxSearchDepth = 32;

    struct SearchInfo {
        u64 nodes_searched = 0;
        int seldepth = 0;
    };

    struct RootSearchResult {
        int score = 0;
        u16 move;
    };

    enum NodeType : u8 {
        RootNode,
        PVNode,
        NonPVNode
    };

    void start(Position pos, int depth, int movetime);
    inline void stop() { Timer::request_stop(); }

    template <NodeType NT>
    int search(SearchInfo& info, Position& pos, int depth, int plies_from_root, int alpha, int beta, bool allow_nmp = true);
    int qsearch_node(SearchInfo& info, Position& pos, int depth, int plies_from_root, int alpha, int beta);

    void init();
}