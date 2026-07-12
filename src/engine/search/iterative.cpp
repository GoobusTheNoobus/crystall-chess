#include "engine/search/search.hpp"
#include "engine/timer.hpp"
#include "engine/tt/tt.hpp"
#include "protocol/uci.hpp"

namespace Crystall::Search {

    constexpr int AspirationWindow = 130;
    constexpr int AspirationExpansion = 2;

    void run_iterative_deepening(Position pos, int max_depth, int movetime) {

        Timer::start(movetime);

        Move best_move;
        int score = 0;

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

        std::cout << "bestmove " << best_move.to_string() << std::endl;

    }
}