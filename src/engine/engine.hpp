#pragma once

#include "chess/board/position.hpp"

namespace Crystall::Engine {

    void start(const Position& pos, int depth, int movetime);
    void stop();

}