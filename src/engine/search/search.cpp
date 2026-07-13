#include "engine/search/search.hpp"
#include "protocol/uci.hpp"
#include "chess/move/movelist.hpp"
#include "engine/tt/tt.hpp"
#include "engine/timer.hpp"
#include "engine/search/history.hpp"

#include <chrono>
#include <atomic>
#include <algorithm>
#include <cstring>

namespace Crystall::Search {

    namespace {

        int reduction_table[2][256][MaxSearchDepth];

        constexpr int AspirationWindow = 130;
        constexpr int MaxQSearchDepth = 15;
        constexpr int MinNMPDepth = 3;
        constexpr int NMPReduction = 2;
    }

    void init() {
        for (int m = 1; m < 256; ++m) {
            for (int d = 1; d < MaxSearchDepth; ++d) {
                reduction_table[false][m][d] = 0.18 * std::log(m) * std::log(d) + 0.48;
                reduction_table[true][m][d]  = 0.12 * std::log(m) * std::log(d) + 0.42;
            }
        }
    }

    RootSearchResult search_root(SearchInfo& info, Position& pos, int depth, int alpha, int beta, const Move& root_pv, bool log_currmove) {
        int best_score = NegativeInfinity;
        Move best_move;

        MoveList moves(pos);
        moves.calculate_scores(root_pv);

        int i = 0;
        int legal_moves = 0;
        while (moves.next(i)) {
            Move move = moves[i];
            ++i;

            bool is_legal = pos.attempt_move(move);
            if (!is_legal) continue;

            ++legal_moves;

            if (log_currmove) {
                UCI::info_depth(depth, Timer::elapsed(), move, legal_moves);
            }

            int score;
            if (legal_moves == 1) {
                score = -search_node<true>(info, pos, depth - 1, -beta, -alpha);
            }
            else {
                score = -search_node<false>(info, pos, depth - 1, -beta, -alpha);
            }
                
            pos.undo_move();

            if (Timer::should_stop_search()) break;

            if (score > best_score) {
                best_move = move;
                best_score = score;
            }

            alpha = std::max(alpha, best_score);
            if (alpha >= beta) break;
        }

        return {best_move, best_score};
    }

    template<bool is_pv>
    int search_node(SearchInfo& info, Position& pos, int depth, int alpha, int beta, bool allow_nmp) {
        ++info.nodes_searched;

        if (pos.is_repetition() || pos.is_rule_50()) return DrawScore;
        if (depth == 0) {
            int qsearch_depth = info.plies_from_root * 2 + 2;
            return qsearch_node(info, pos, std::min(qsearch_depth, MaxQSearchDepth), alpha, beta);
        }

        if (info.nodes_searched % 100000 == 0) {
            History::clear();
        }

        ++info.plies_from_root;
        info.seldepth = std::max(info.seldepth, info.plies_from_root);

        if (Timer::should_stop_search()) return Timeout;

        auto entry = TranspositionTable::read(pos.get_key());
        Move tt_move;
        if (entry.depth != 0 && entry.key == pos.get_key()) {

            tt_move = entry.best_move;

            if (entry.depth >= depth) {
                    if (entry.flag == TranspositionTable::Exact) {
                    --info.plies_from_root;
                    return entry.score;
                }
                    
                else if (entry.flag == TranspositionTable::Lower) alpha = std::max(alpha, entry.score);
                else if (entry.flag == TranspositionTable::Upper) beta = std::min(beta, entry.score);

                if (alpha >= beta) {
                    --info.plies_from_root;
                    return entry.score;
                }
            }
            
        }

        bool in_check = pos.is_in_check();
        int static_eval = pos.evaluate();
        
        if (!in_check && !is_pv && depth <= 3 && static_eval - 80 * depth >= beta) {
            --info.plies_from_root;
            return static_eval;
        }

        if (!is_pv && !in_check && allow_nmp && depth >= MinNMPDepth && static_eval >= beta && pos.has_non_pawn_material()) {

            int reduction = NMPReduction;

            pos.make_move(Move::NullMove);
            int null_score = -search_node<false>(info, pos, depth - 1 - reduction, -beta, -beta + 1, false);
            pos.undo_move();

            if (null_score >= beta) {
                --info.plies_from_root;
                return beta;
            }
        }

        int best_score = NegativeInfinity;
        Move best_move;
        int legal_moves = 0;
        int original_alpha = alpha;

        MoveList moves(pos);
        moves.calculate_scores(tt_move);

        int i = 0;
        while (moves.next(i)) {
            Move move = moves[i];
            ++i;

            bool is_legal = pos.attempt_move(move);
            if (!is_legal) continue;

            ++legal_moves;

            int score;
            if (legal_moves == 1) {
                score = -search_node<is_pv>(info, pos, depth - 1, -beta, -alpha, false);
            } 
            else {

                score = -search_node<false>(info, pos, depth - 1, -alpha - 1, -alpha);

                if (score > alpha && score < beta)
                    score = -search_node<true>(info, pos, depth - 1, -beta, -alpha);
            }

            pos.undo_move();

            alpha = std::max(score, alpha);
            
            if (score > best_score) {
                best_score = score;
                best_move = move;
            }

            if (alpha >= beta) {
                if (!is_noisy(pos, move)) {
                    History::table[pos.get_side_to_move()][move.from()][move.dest()] += depth * depth;
                }
                break;
            }
        }

        if (legal_moves == 0) {
            --info.plies_from_root;
            if (pos.is_in_check()) 
                return -MateScore + info.plies_from_root + 1;
            return DrawScore;
        }

        TranspositionTable::EntryType store_flag;
        if (best_score <= original_alpha) store_flag = TranspositionTable::Upper;
        else if (best_score >= beta) store_flag = TranspositionTable::Lower;
        else store_flag = TranspositionTable::Exact;

        if (best_move.is_valid()) {
            TranspositionTable::write(pos.get_key(), best_move, std::clamp(best_score, -KnownWin, KnownWin), depth, store_flag);
        }

        --info.plies_from_root;
        return best_score;
    }
}