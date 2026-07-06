#include "search.hpp"
#include "uci.hpp"
#include "movelist.hpp"
#include "tt.hpp"

#include <chrono>
#include <atomic>
#include <algorithm>
#include <cstring>

namespace Crystall::Search {

    void clear_history_table() {
        std::memset(history_table, 0, sizeof(history_table));
    }

    namespace {

        constexpr int AspirationWindow = 130;
        constexpr int MaxQSearchDepth = 15;
        constexpr int MinNMPDepth = 3;
        constexpr int NMPReduction = 2;

        // declare functions
        template <bool is_pv>
        int search_node(SearchInfo& info, Position& pos, int depth, int alpha, int beta, bool allow_nmp = true);
        int qsearch_node(SearchInfo& info, Position& pos, int depth, int alpha, int beta);
        RootSearchResult search_root(SearchInfo& info, Position& pos, int depth, int alpha, int beta, const Move& move, bool log_currmove);

        using Time = std::chrono::steady_clock::time_point;

        struct Timer {
            private:
            inline static Time start_time;
            inline static int max_time_ms;
            inline static std::atomic_bool stop_flag;

            public:
            inline static void start(int max_time) {
                max_time_ms = max_time;
                start_time = std::chrono::steady_clock::now();
                stop_flag.store(false);
            }

            static void request_stop() {
                stop_flag.store(true);
            }

            inline static int elapsed() {
                return std::chrono::duration_cast<std::chrono::milliseconds>
                       (std::chrono::steady_clock::now() - start_time).count();
            }

            inline static bool should_stop_search() {
                return stop_flag.load() || (max_time_ms > 0 && elapsed() > max_time_ms);
            }
        };

        bool is_noisy(const Position& pos, const Move& move) {
            return move.flag() >= Move::EnPassant || pos.get_piece_on(move.dest()) != NoPiece;
        }

        void normalize_history_table() {
            for (int c = 0; c < ColorNB; ++c) {
                for (int from = 0; from < SquareNB; ++from) {
                    for (int dest = 0; dest < SquareNB; ++dest) {
                        history_table[c][from][dest] /= 2;
                    }
                }
            }
        }
    }

    void start(Position pos, int max_depth, int movetime) {

        Timer::start(movetime);

        Move best_move;
        int score = 0;

        // Iterative Deepening
        SearchInfo info;
        for (int depth = 1; depth <= max_depth; ++depth) {

            bool log_currmove = Timer::elapsed() > 500;

            info.seldepth = 0;
            info.previous_nodes_searched = info.nodes_searched;

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

                    // fail low
                    if (result.score <= alpha) {
                        alpha -= delta;
                        delta *= 2;
                        continue;
                    }

                    // fail high
                    if (result.score >= beta) {
                        beta += delta;
                        delta *= 2;
                        continue;
                    }

                    // inside window
                    break;
                }
            }
            
            if (Timer::should_stop_search()) break;

            best_move = result.move;
            score = result.score;

            Position copy = pos;
            std::vector<Move> pv;

            if (best_move.is_valid() && copy.attempt_move(best_move)) {
                pv.push_back(best_move);
            }
            
            for (int i = 0; i < depth; ++i) {
                auto entry = TranspositionTable::read(copy.get_key());
                if (entry.depth > 0 && entry.flag == TranspositionTable::Exact) {
                    Move move = entry.best_move;

                    if (!copy.attempt_move(move)) break;

                    pv.push_back(move);
                }
            }

            UCI::info_depth(
                depth, 
                info.seldepth, 
                result.score, 
                info.nodes_searched - info.previous_nodes_searched, 
                Timer::elapsed(), 
                info.nodes_searched, 
                pv
            );
        }

        TranspositionTable::clear();
        std::cout << "bestmove " << best_move.to_string() << std::endl;

    }

    void stop() {
        Timer::request_stop();
    }

    namespace {

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
                    UCI::info_depth(depth, move, legal_moves);
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
                if (alpha >= beta) break; // not possible yet, since beta is passed as infinity
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
                normalize_history_table();
            }

            ++info.plies_from_root;
            info.seldepth = std::max(info.seldepth, info.plies_from_root);

            if (Timer::should_stop_search()) return Timeout;

            // tt probe

            auto entry = TranspositionTable::read(pos.get_key());
            Move tt_move;
            if (entry.depth != 0 && entry.depth >= depth && entry.key == pos.get_key()) {
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

                tt_move = entry.best_move;
            }

            bool in_check = pos.is_in_check();
            int static_eval = pos.evaluate();

            // nmp
            if (!is_pv && !in_check && allow_nmp && depth >= MinNMPDepth && static_eval >= beta && pos.has_non_pawn_material()) {

                int reduction = NMPReduction; // TODO: make it a formula

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
                if (legal_moves == 1 && is_pv) {
                    // we dont allow nmp 
                    score = -search_node<true>(info, pos, depth - 1, -beta, -alpha, false);
                } else {

                    score = -search_node<false>(info, pos, depth - 1, -alpha - 1, -alpha);

                    if (score > alpha && score < beta)
                        score = -search_node<false>(info, pos, depth - 1, -beta, -alpha);
                }

                pos.undo_move();

                alpha = std::max(score, alpha);
                
                if (score > best_score) {
                    best_score = score;
                    best_move = move;
                }

                // alpha beta pruning
                if (alpha >= beta) {
                    if (!is_noisy(pos, move)) {
                        history_table[pos.get_side_to_move()][move.from()][move.dest()] += depth * depth;
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

        int qsearch_node(SearchInfo& info, Position& pos, int depth, int alpha, int beta) {
            ++info.nodes_searched;

            if (pos.is_repetition() || pos.is_rule_50()) return DrawScore;
            if (Timer::should_stop_search()) return Timeout;

            bool in_check = pos.is_in_check();
            int static_eval = pos.evaluate();

            // if we are not in check and qsearch budget is exhausted, standpat
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
}