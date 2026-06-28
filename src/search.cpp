#include "search.hpp"
#include "uci.hpp"
#include "movelist.hpp"
#include <chrono>
#include <atomic>

namespace Crystall::Search {

    namespace {

        // declare functions
        int search_node(SearchInfo& info, Position& pos, int depth, int alpha, int beta);
        int qsearch_node(SearchInfo& info, Position& pos, int depth, int alpha, int beta);
        RootSearchResult search_root(SearchInfo& info, Position& pos, int depth, int alpha, int beta);

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
    }

    void start(Position pos, int max_depth, int movetime) {

        UCI::info_string("starting search with depth " + std::to_string(max_depth));
        UCI::info_string("starting search with movetime " + std::to_string(movetime) + "ms");

        Timer::start(movetime);

        Move best_move;

        SearchInfo info;
        for (int depth = 1; depth <= max_depth; ++depth) {

            info.seldepth = 0;
            info.previous_nodes_searched = info.nodes_searched;

            RootSearchResult result;
            result = search_root(info, pos, depth, -Infinity, Infinity);

            if (Timer::should_stop_search()) break;

            best_move = result.move;

            UCI::info_depth(
                depth, 
                info.seldepth, 
                result.score, 
                info.nodes_searched - info.previous_nodes_searched, 
                Timer::elapsed(), 
                info.nodes_searched, 
                best_move
            );
        }

        std::cout << "bestmove " << best_move.to_string() << std::endl;
    }

    void stop() {
        Timer::request_stop();
    }

    namespace {

        RootSearchResult search_root(SearchInfo& info, Position& pos, int depth, int alpha, int beta) {
            int best_score = NegativeInfinity;
            Move best_move;

            MoveList moves(pos);
            for (int i = 0; i < moves.size(); ++i) {
                Move move = moves[i];

                bool is_legal = pos.attempt_move(move);
                if (!is_legal) continue;

                int score = -search_node(info, pos, depth - 1, -beta, -alpha);
                pos.undo_move();

                if (score > best_score) {
                    best_move = move;
                    best_score = score;
                }

                alpha = std::max(alpha, best_score);
                if (alpha >= beta) break; // not possible yet, since beta is passed as infinity
            }

            return {best_move, best_score};
        }

        int search_node(SearchInfo& info, Position& pos, int depth, int alpha, int beta) {
            ++info.nodes_searched;

            if (pos.is_repetition() || pos.is_rule_50()) return DrawScore;
            if (depth == 0) {
                int qsearch_depth = info.plies_from_root * 2 + 2;
                return qsearch_node(info, pos, std::min(qsearch_depth, 15), alpha, beta);
            }

            ++info.plies_from_root;
            info.seldepth = std::max(info.seldepth, info.plies_from_root);

            if (Timer::should_stop_search()) return Timeout;

            int best_score = NegativeInfinity;
            int legal_moves = 0;

            MoveList moves(pos);
            for (int i = 0; i < moves.size(); ++i) {
                Move move = moves[i];

                bool is_legal = pos.attempt_move(move);
                if (!is_legal) continue;

                ++legal_moves;

                int score = -search_node(info, pos, depth - 1, -beta, -alpha);
                pos.undo_move();

                alpha = std::max(score, alpha);
                best_score = std::max(score, best_score);

                // alpha beta pruning
                if (alpha >= beta) break;
            }

            --info.plies_from_root;

            if (legal_moves == 0) {
                if (pos.is_in_check()) 
                    return -MateScore + info.plies_from_root + 1;
                return DrawScore;
            }

            return best_score;
        }

        bool is_noisy(const Position& pos, const Move& move) {
            return move.flag() >= Move::EnPassant || pos.get_piece_on(move.dest()) != NoPiece;
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