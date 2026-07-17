// Crystall is a hobby UCI chess engine written in C++
// Developed by GoobusTheNoobus

#pragma once

#include "move.hpp"
#include "position.hpp"

namespace Crystall::MoveGen {
    int generate_pseudo_legal_moves(const Position& pos, Move moves[]);
}
