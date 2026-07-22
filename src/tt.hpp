// Crystall is a hobby UCI chess engine written in C++
// Developed by GoobusTheNoobus

#pragma once

#include "types.hpp"
#include "move.hpp"
#include <iostream>

namespace Crystall {

    namespace TranspositionTable {

        constexpr int BucketSize = 4;

        enum EntryType : u8 {
            Exact,
            Lower,
            Upper
        };

        struct Entry {
            u64 key = 0;
            int score = 0;
            u16 best_move;
            u8 depth = 0;
            EntryType flag = Exact;
        };

        struct Bucket {
            Entry entries[BucketSize];
        };

        namespace {
            inline constexpr static int TableMB = 108;
            inline constexpr static int BucketCount = (TableMB * 1024 * 1024) / sizeof(Bucket);

            inline static Bucket data[BucketCount];
        }

        void write(u64 key, u16 best_move, int score, u8 depth, EntryType flag);
        const Bucket& probe(u64 key);
        void clear();

        int hashfull();
    };
}