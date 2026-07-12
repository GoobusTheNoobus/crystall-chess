#pragma once

#include "chess/types.hpp"
#include "chess/board/position.hpp"
#include "engine/timer.hpp"

namespace Crystall::Search {
    constexpr int MaxSearchDepth = 32;

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

    void run_iterative_deepening(Position pos, int depth, int movetime);
    inline void stop() { Timer::request_stop(); }

    template <bool is_pv>
    int search_node(SearchInfo& info, Position& pos, int depth, int alpha, int beta, bool allow_nmp = true);
    int qsearch_node(SearchInfo& info, Position& pos, int depth, int alpha, int beta);
    RootSearchResult search_root(SearchInfo& info, Position& pos, int depth, int alpha, int beta, const Move& move, bool log_currmove);

    inline bool is_noisy(const Position& pos, const Move& move) {
        return move.flag() >= Move::EnPassant || pos.get_piece_on(move.dest()) != NoPiece;
    }

    void init();
}