// Crystall is a hobby UCI chess engine written in C++
// Developed by GoobusTheNoobus

#pragma once

#include "types.hpp"
#include "move.hpp"
#include <iostream>

namespace Crystall {

    namespace TranspositionTable {

        enum EntryType : uint8_t {
            Exact,
            Lower,
            Upper
        };

        struct Entry {
            u64 key = 0;
            Move best_move;
            int score = 0;
            uint8_t depth = 0;
            EntryType flag = Exact;

        };

        namespace {
            inline constexpr static int TableMB = 108;
            inline constexpr static int EntriesNB = (TableMB * 1024 * 1024) / sizeof(Entry);
            
            inline static Entry data[EntriesNB];

            static inline int get_index(u64 key) { return key % EntriesNB; }
        }

        void write(u64 key, Move& best_move, int score, uint8_t depth, EntryType flag);
        const Entry& read(u64 key);
        void clear();

        int hashfull();
        
        const static Entry NullEntry = {0, Move::NullMove, 0, 0, Exact};
    };
}