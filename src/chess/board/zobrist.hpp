#pragma once

#include "chess/types.hpp"

namespace Crystall::Zobrist {

    inline u64 PieceSquareKeys[PieceNB][SquareNB];
    inline u64 CastlingKeys[16];
    inline u64 EnPassantKeys[8];
    inline u64 SideKey;

    void init();
}