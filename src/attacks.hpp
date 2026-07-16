#pragma once

#include "position.hpp"

namespace Crystall::Attack {
    bool is_attacked(const Position& pos, Square square, Color by);
}
