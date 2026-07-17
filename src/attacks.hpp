// Crystall is a hobby UCI chess engine written in C++
// Developed by GoobusTheNoobus

#pragma once

#include "position.hpp"

namespace Crystall::Attack {
    bool is_attacked(const Position& pos, Square square, Color by);
}
