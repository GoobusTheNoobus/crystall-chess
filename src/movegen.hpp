#pragma once

#include "move.hpp"
#include "position.hpp"

namespace Crystall::MoveGen {
    int generate_pseudo_legal_moves(const Position& pos, Move moves[]);
}
