// Crystall is a hobby UCI chess engine written in C++
// Developed by GoobusTheNoobus

#include "zobrist.hpp"
#include <random>

void Crystall::Zobrist::init() {
    std::mt19937_64 rng(67);

    for (int i = 0; i < PieceNB; ++i) {
        for (int j = 0; j < SquareNB; ++j) {
            PieceSquareKeys[i][j] = rng();
        }
    }

    for (int i = 0; i < 16; ++i) {
        CastlingKeys[i] = rng();
    }

    for (int i = 0; i < 8; ++i) {
        EnPassantKeys[i] = rng();
    }

    SideKey = rng();

}