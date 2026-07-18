// Crystall is a hobby UCI chess engine written in C++
// Developed by GoobusTheNoobus

#include "search.hpp"
#include "uci.hpp"
#include "movelist.hpp"
#include "tt.hpp"
#include "timer.hpp"
#include "history.hpp"

#include <chrono>
#include <atomic>
#include <algorithm>
#include <cstring>

namespace Crystall::Search {

    namespace {

        int reduction_table[256][MaxSearchDepth];

        constexpr int AspirationWindow = 130;
        constexpr int MaxQSearchDepth = 15;
        constexpr int MinNMPDepth = 3;
        constexpr int NMPReduction = 2;
    }

    void init() {
        for (int m = 1; m < 256; ++m) {
            for (int d = 1; d < MaxSearchDepth; ++d) {
                reduction_table[m][d] = (int)(0.5 * std::log(m) * std::log(d) + 0.4);
            }
        }
    }

    constexpr int AspirationExpansion = 2;

    void start(Position pos, int max_depth, int movetime) {

        TranspositionTable::clear();
        Timer::start(movetime);

        u16 best_move = Move::NullMove;
        int score = 0;

        SearchInfo info;
        for (int depth = 1; depth <= max_depth; ++depth) {

            bool log_currmove = Timer::elapsed() > 500;

            info.seldepth = 0;

            int delta = AspirationWindow;

            int alpha = score - delta;
            int beta = score + delta;

            RootSearchResult result;

            if (depth == 1) {
                result = search_root(info, pos, depth, alpha, beta, best_move, false);
            }
            else {
                while (true) {
                    result = search_root(info, pos, depth, alpha, beta, best_move, log_currmove);

                    if (Timer::should_stop_search()) break;

                    if (result.score <= alpha) {
                        alpha -= delta;
                        delta *= AspirationExpansion;
                        continue;
                    }

                    if (result.score >= beta) {
                        beta += delta;
                        delta *= AspirationExpansion;
                        continue;
                    }

                    break;
                }
            }

            if (Timer::should_stop_search()) break;

            best_move = result.move;
            score = result.score;

            Position copy = pos;
            std::vector<u16> pv;

            if (copy.attempt_move(best_move)) {
                pv.push_back(best_move);
            }

            for (int i = 0; i < depth; ++i) {
                auto entry = TranspositionTable::read(copy.get_key());
                if (entry.depth > 0 && entry.flag == TranspositionTable::Exact) {
                    u16 move = entry.best_move;

                    if (!copy.attempt_move(move)) break;

                    pv.push_back(move);
                }
            }

            UCI::info_depth(depth, info.seldepth, result.score, Timer::elapsed(), info.nodes_searched, pv);
        }

        std::cout << "bestmove " << Move::to_string(best_move) << std::endl;
    }

    RootSearchResult search_root(SearchInfo& info, Position& pos, int depth, int alpha, int beta, const u16 root_pv, bool log_currmove) {
        int best_score = NegativeInfinity;
        u16 best_move = Move::NullMove;

        MoveList moves(pos);
        moves.calculate_scores(root_pv);

        int i = 0;
        int legal_moves = 0;
        while (moves.next(i)) {
            u16 move = moves[i];
            ++i;

            bool is_legal = pos.attempt_move(move);
            if (!is_legal) continue;

            ++legal_moves;

            if (log_currmove) {
                UCI::info_depth(depth, Timer::elapsed(), info.nodes_searched, move, legal_moves);
            }

            int score;
            if (legal_moves > 1) {
                score = -search_node<false>(info, pos, depth - 1, -alpha - 1, -alpha);
            }

            if (legal_moves == 1 || score > alpha) {
                score = -search_node<true>(info, pos, depth - 1, -beta, -alpha);
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

        return {best_score, best_move};
    }

    template<bool is_pv>
    int search_node(SearchInfo& info, Position& pos, int depth, int alpha, int beta, bool allow_nmp) {
        ++info.nodes_searched;

        if (pos.is_repetition() || pos.is_rule_50()) return DrawScore;
        if (depth == 0) {
            int qsearch_depth = info.plies_from_root * 2 + 2;
            return qsearch_node(info, pos, std::min(qsearch_depth, MaxQSearchDepth), alpha, beta);
        }

        ++info.plies_from_root;
        info.seldepth = std::max(info.seldepth, info.plies_from_root);

        if (Timer::should_stop_search()) return Timeout;

        auto entry = TranspositionTable::read(pos.get_key());
        u16 tt_move = Move::NullMove;
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
        u16 best_move = Move::NullMove;
        int legal_moves = 0;
        int original_alpha = alpha;

        MoveList moves(pos);
        moves.calculate_scores(tt_move);

        int i = 0;
        while (moves.next(i)) {
            u16 move = moves[i];
            ++i;

            bool noisy = is_noisy(pos, move);

            bool is_legal = pos.attempt_move(move);
            if (!is_legal) continue;

            ++legal_moves;


            int score;
            if (!is_pv || legal_moves > 1) {
                score = -search_node<false>(info, pos, depth - 1, -alpha - 1, -alpha);
            }

            if (is_pv && (legal_moves == 1 || score > alpha)) {
                score = -search_node<true>(info, pos, depth - 1, -beta, -alpha);
            }

            pos.undo_move();

            if (score >= alpha) {
                alpha = score;
            }

            if (score > best_score) {
                best_score = score;
                best_move = move;
            }

            if (alpha >= beta) {
                if (!noisy) {
                    History::update(pos.get_side_to_move(), Move::from(move), Move::dest(move), std::min(300 * depth - 300, 2500));

                    // Loop all previously searched moves to penalise
                    for (int j = 0; j < i; ++j) {
                        u16 m = moves[j];
                        if (!is_noisy(pos, m))
                            History::update(pos.get_side_to_move(), Move::from(m), Move::dest(m), -std::min(300 * depth - 300, 2500));
                    }
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

        if (best_move != 0) {
            TranspositionTable::write(pos.get_key(), best_move, std::clamp(best_score, -KnownWin, KnownWin), depth, store_flag);
        }

        --info.plies_from_root;
        return best_score;
    }

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
            u16 move = moves[i];

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