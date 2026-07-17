// Crystall is a hobby UCI chess engine written in C++
// Developed by GoobusTheNoobus

#pragma once

#include "types.hpp"
#include <cstring>
#include <algorithm>

namespace Crystall::Search {

    namespace History {
        inline int table[ColorNB][SquareNB][SquareNB];

        inline void clear() {
            std::memset(table, 0, sizeof(table));
        }

        inline void normalize() {
            for (int c = 0; c < ColorNB; ++c) {
                for (int from = 0; from < SquareNB; ++from) {
                    for (int dest = 0; dest < SquareNB; ++dest) {
                        table[c][from][dest] /= 2;
                    }
                }
            }
        }

        inline void update(Color c, Square f, Square d, int bonus) {
            int clamped = std::clamp(bonus, -500000, 500000);
            table[c][f][d] += clamped - table[c][f][d] * abs(clamped) / 500000;
        }
    }
}