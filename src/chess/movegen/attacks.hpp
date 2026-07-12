#pragma once

#include "chess/board/position.hpp"

namespace Crystall::Attack {
    bool is_attacked(const Position& pos, Square square, Color by);
}
