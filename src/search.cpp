#include "search.hpp"
#include "movelist.hpp"
#include "uci.hpp"

#include <chrono>
#include <iostream>
#include <atomic>

using namespace std::chrono;

namespace Crystall::Search {

    using TimePoint = steady_clock::time_point;

    struct SearchOptions {
        constexpr static int DeltaPruningMargin = 140;
        constexpr static int MaxQSearchDepth = 15;
    };

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
        int seldepth = 0;
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

    bool is_noisy(Position& pos, Move move) {
        Move::Type flag = move.flag();

        if (flag >= Move::PromoQ || flag == Move::EnPassant) return true;

        return pos.get_piece_on(move.dest()) != NoPiece;
    }

    int qsearch_node(SearchInfo& info, Position& pos, int depth, int alpha, int beta) {
        info.nodes_searched++;

        if (pos.is_repetition())
            return DrawScore;

        bool in_check = pos.is_in_check();
        int static_eval = pos.evaluate();

        if (!in_check) {
            if (static_eval >= beta)
                return static_eval;
            alpha = std::max(alpha, static_eval);
        }

        if (depth <= 0 && !in_check)
            return static_eval;

        info.plies_from_root++;
        info.seldepth = std::max(info.seldepth, info.plies_from_root);

        MoveList moves(pos);
        bool has_legal_move = false;

        for (int i = 0; i < moves.size(); ++i) {
            Move move = moves[i];

            Move::Type flag = move.flag();
            Piece victim = pos.get_piece_on(move.dest());

            bool is_ep = flag == Move::EnPassant;
            bool is_promo = flag >= Move::PromoQ;
            bool is_capture = victim != NoPiece || is_ep;
            bool noisy = is_capture || is_promo;

            if (!in_check && !noisy)
                continue;

            if (!in_check && !is_promo) {
                int gain = 0;
                if (is_ep)
                    victim = make_piece(Pawn, opposite(pos.get_side_to_move()));
                if (is_capture)
                    gain = Evaluation::MGValues[type_of(victim)];

                if (static_eval + gain + SearchOptions::DeltaPruningMargin < alpha)
                    continue;
            }

            bool is_legal = pos.attempt_move(move);
            if (!is_legal)
                continue;

            has_legal_move = true;

            int score = -qsearch_node(info, pos, depth - 1, -beta, -alpha);

            pos.undo_move();

            if (score >= beta) {
                info.plies_from_root--;
                return score;
            }

            alpha = std::max(alpha, score);
        }

        if (in_check && !has_legal_move) {
            info.plies_from_root--;
            return -(MateScore - (info.plies_from_root + 1));
        }

        info.plies_from_root--;
        return alpha;
    }

    int search_node(SearchInfo& info, Position& pos, int depth, int alpha, int beta) {
        info.nodes_searched++;

        if (SearchTimer::timeout_search())
            return Timeout;

        info.plies_from_root++;
        info.seldepth = std::max(info.seldepth, info.plies_from_root);

        if (pos.is_repetition() || pos.is_rule_50()) {
            info.plies_from_root--;
            return DrawScore;
        }

        if (depth <= 0) {
            
            int score = qsearch_node(info, pos, SearchOptions::MaxQSearchDepth, alpha, beta);
            info.plies_from_root--;
            return score;
        }

        MoveList moves(pos);
        int best_score = -Infinity;
        bool has_legal_move = false;

        for (int i = 0; i < moves.size(); ++i) {
            Move move = moves[i];

            bool is_legal = pos.attempt_move(move);
            if (!is_legal) continue;

            has_legal_move = true;

            int score = -search_node(info, pos, depth - 1, -beta, -alpha);

            pos.undo_move();

            best_score = std::max(best_score, score);
            alpha = std::max(alpha, score);

            if (alpha >= beta)
                break;
        }

        int result;
        if (!has_legal_move) {
            Color us = pos.get_side_to_move();
            result = pos.is_in_check(us) ? -(MateScore - info.plies_from_root) : DrawScore;
        } else {
            result = best_score;
        }

        info.plies_from_root--;
        return result;
    }

    RootSearchResult search_root(SearchInfo& info, Position& pos, int depth) {
        MoveList moves(pos);
        int best_score = -Infinity;
        Move best_move;

        for (int i = 0; i < moves.size(); ++i) {

            Move move = moves[i];

            bool is_legal = pos.attempt_move(move);
            if (!is_legal) continue;

            if (info.plies_from_root != 0) {
                std::cerr << "Plies from root is non zero at root\n";
                std::cerr << "Plies from root: " << info.plies_from_root << std::endl;
                std::exit(-1);
            }

            int score = -search_node(info, pos, depth - 1, -Infinity, Infinity);

            pos.undo_move();

            if (score > best_score) {
                best_score = score;
                best_move = move;
            }

            if (SearchTimer::timeout_search()) break;
        }

        return {best_score, best_move};
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

        UCI::info_string("score cp " + std::to_string(pos.evaluate()));

        for (int depth = 1; depth <= max_depth; ++depth) {
            info.previous_nodes_searched = info.nodes_searched;
            info.seldepth = 0;

            RootSearchResult result = search_root(info, pos, depth);

            if (SearchTimer::timeout_search()) break;

            best_move = result.move;

            UCI::info_depth(depth, info.seldepth, result.score, info.nodes_searched - info.previous_nodes_searched, SearchTimer::elapsed_ms(), info.nodes_searched, result.move);
        }

        std::cout << "bestmove " << best_move.to_string() << std::endl;
    }

    void stop_search() {
        SearchTimer::stop_flag = true;
    }
}
