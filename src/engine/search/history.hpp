#pragma once

#include "chess/types.hpp"
#include <cstring>

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
    }

    namespace Butterfly {

        constexpr int MaxButterflyScore = 6700;
        inline int table[ColorNB][SquareNB][SquareNB];

        inline void clear() {
            std::memset(table, 0, sizeof(table));
        }

        inline void add(Color c, Square f, Square d) {
            ++table[c][f][d];
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
    }
}