#pragma once

#include "chess/types.hpp"
#include "chess/move/move.hpp"
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

            static const Entry NullEntry;
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
        
    };
}