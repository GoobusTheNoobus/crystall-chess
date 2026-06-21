#pragma once

#include "types.hpp"
#include "position.hpp"

namespace Crystall::Search {
    void start_search(Position& pos, int depth, int movetime);
    void stop_search();

    void perft_divide(Position& pos, int depth);
}