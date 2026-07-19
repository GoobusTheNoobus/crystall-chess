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
        int plies_from_root = 0;
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

    template <bool is_pv>
    int search_node(SearchInfo& info, Position& pos, int depth, int alpha, int beta, bool allow_nmp = true);
    int qsearch_node(SearchInfo& info, Position& pos, int depth, int alpha, int beta);
    RootSearchResult search_root(SearchInfo& info, Position& pos, int depth, int alpha, int beta, const u16 move, bool log_currmove);

    inline bool is_noisy(const Position& pos, const u16 move) {
        return Move::type(move) >= Move::EnPassant || pos.get_piece_on(Move::dest(move)) != NoPiece;
    }

    void init();
}