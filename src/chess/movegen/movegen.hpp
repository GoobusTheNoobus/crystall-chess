#pragma once

#include "chess/move/move.hpp"
#include "chess/board/position.hpp"

namespace Crystall::MoveGen {
    int generate_pseudo_legal_moves(const Position& pos, Move moves[]);
}
