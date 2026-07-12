#include "engine/engine.hpp"
#include "engine/search/tt.hpp"
#include "engine/search/timer.hpp"
#include "engine/search/search.hpp"

namespace Crystall::Engine {

    void start(const Position& pos, int depth, int movetime) {
        TranspositionTable::clear();
        Search::Timer::start(movetime);

        Search::run_iterative_deepening(pos, depth, movetime);
    }

    void stop() {
        Search::Timer::request_stop();
    }
}