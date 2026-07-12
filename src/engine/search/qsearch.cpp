#include "engine/search/search.hpp"
#include "engine/timer.hpp"
#include "chess/move/movelist.hpp"

namespace Crystall::Search {

    int qsearch_node(SearchInfo& info, Position& pos, int depth, int alpha, int beta) {
        ++info.nodes_searched;

        if (pos.is_repetition() || pos.is_rule_50()) return DrawScore;
        if (Timer::should_stop_search()) return Timeout;

        bool in_check = pos.is_in_check();
        int static_eval = pos.evaluate();

        if (depth <= 0 && !in_check) return static_eval;

        if (!in_check && static_eval >= beta) {
            return static_eval;
        }

        alpha = std::max(alpha, static_eval);

        ++info.plies_from_root;
        info.seldepth = std::max(info.seldepth, info.plies_from_root);

        MoveList moves(pos);
        for (int i = 0; i < moves.size(); ++i) {
            Move move = moves[i];

            bool search_move = in_check || is_noisy(pos, move);
            if (!search_move) continue;

            bool is_legal = pos.attempt_move(move);
            if (!is_legal) continue;

            int score = -qsearch_node(info, pos, depth - 1, -beta, -alpha);
            pos.undo_move();

            if (score > alpha) {
                alpha = score;
                if (alpha >= beta) {
                    --info.plies_from_root;
                    return beta;
                }
            }
        }

        --info.plies_from_root;

        return alpha;
    }
}