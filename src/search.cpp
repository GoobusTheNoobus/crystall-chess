#include "search.hpp"
#include "movelist.hpp"
#include "uci.hpp"

#include <chrono>
#include <iostream>
#include <atomic>

using namespace std::chrono;

namespace Crystall::Search {

    namespace {

        using TimePoint = steady_clock::time_point;

        struct SearchTimer {
            inline static TimePoint start_time;
            inline static int max_time_ms;
            inline static std::atomic_bool stop_flag{false};

            static bool timeout_search() {
                if (stop_flag) return true;

                // if the max time is 0, the search is infinite
                if (max_time_ms <= 0) return false;

                TimePoint now = steady_clock::now();
                if (duration_cast<milliseconds>(now - start_time).count() > max_time_ms) return true;

                return false;
            }

            static void start(int movetime) {
                stop_flag = false;
                max_time_ms = movetime;
                start_time = steady_clock::now();
            }

            static int elapsed_ms() {
                TimePoint now = steady_clock::now();

                return duration_cast<milliseconds>(now - start_time).count();
            }
        };

        struct SearchInfo {
            u64 nodes_searched = 0;
            u64 previous_nodes_searched = 0;
            int plies_from_root = 0;
        };

        struct RootSearchResult {
            int score = 0;
            Move move;
        };

        int perft(Position& pos, int depth_left) {
            if (depth_left <= 0) return 1;

            int nodes = 0;

            MoveList moves(pos);
            for (int i = 0; i < moves.size(); ++i) {
                Move move = moves[i];

                bool is_legal = pos.attempt_move(move);
                if (!is_legal) continue;

                nodes += perft(pos, depth_left - 1);

                pos.undo_move();
            }

            return nodes;
        }

        int search_node(SearchInfo& info, Position& pos, int depth, int alpha, int beta) {
            info.nodes_searched++;

            if (depth == 0) {
                return pos.evaluate();
            }

            info.plies_from_root++;

            if (SearchTimer::timeout_search()) return TIMEOUT;

            MoveList moves(pos);
            int best_score = -INF;
            bool has_legal_move = false;

            for (int i = 0; i < moves.size(); ++i) {
                Move move = moves[i];

                bool is_legal = pos.attempt_move(move);
                if (!is_legal) continue;

                has_legal_move = true;
                
                int score = -search_node(info, pos, depth - 1, -beta, -alpha);

                pos.undo_move();

                alpha = std::max(score, alpha);
                best_score = std::max(best_score, score);

                if (alpha >= beta) {
                    break;
                }
            }

            if (!has_legal_move) {
                Color us = pos.get_side_to_move();
                info.plies_from_root--;
                return pos.is_in_check(us) ? -(MATE_SCORE - (info.plies_from_root + 1)) : DRAW_SCORE;
            }

            info.plies_from_root--;

            return best_score;
        }

        RootSearchResult search_root(SearchInfo& info, Position& pos, int depth) {
            MoveList moves(pos);
            int best_score = -INF;
            Move best_move;

            for (int i = 0; i < moves.size(); ++i) {
                Move move = moves[i];

                bool is_legal = pos.attempt_move(move);
                if (!is_legal) continue;

                int score = -search_node(info, pos, depth - 1, -INF, INF);

                pos.undo_move();

                if (score > best_score) {
                    best_score = score;
                    best_move = move;
                }

                if (SearchTimer::timeout_search()) break;
            }

            return {best_score, best_move};
        }

    }

    void perft_divide(Position& pos, int depth) {
        int total_nodes = 0;

        MoveList moves(pos);

        TimePoint start = steady_clock::now();
        for (int i = 0; i < moves.size(); ++i) {
            Move move = moves[i];

            bool is_legal = pos.attempt_move(move);
            if (!is_legal) continue;

            int nodes = perft(pos, depth - 1);
            std::cout << move.to_string() << ": " << nodes << std::endl;
            total_nodes += nodes;

            pos.undo_move();
        }
        TimePoint end = steady_clock::now();

        u64 duration = duration_cast<milliseconds>(end - start).count();

        std::cout << "\nTotal Nodes: " << total_nodes << std::endl;
        std::cout << "Perft completed in " << duration << "ms" << std::endl;
        std::cout << "NPS: " << (total_nodes / duration * 1000) << std::endl; 

    }

    void start_search(Position& pos, int max_depth, int movetime) {
        int score = 0;

        SearchTimer::start(movetime);

        SearchInfo info;

        Move best_move;

        for (int depth = 1; depth <= max_depth; ++depth) {
            info.previous_nodes_searched = info.nodes_searched;

            RootSearchResult result = search_root(info, pos, depth);

            if (SearchTimer::timeout_search()) break;

            best_move = result.move;

            UCI::info_depth(depth, result.score, info.nodes_searched - info.previous_nodes_searched, SearchTimer::elapsed_ms(), info.nodes_searched, result.move);
        }

        std::cout << "bestmove " << best_move.to_string() << std::endl;
    }

    void stop_search() {
        SearchTimer::stop_flag = true;
    }
}
