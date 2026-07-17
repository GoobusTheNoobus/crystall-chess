// Crystall is a hobby UCI chess engine written in C++
// Developed by GoobusTheNoobus

#include "tt.hpp"

#include <cstring>

namespace Crystall {

    const TranspositionTable::Entry& TranspositionTable::read(u64 key) {
        int index = get_index(key);
        Entry& entry = data[index];

        return entry.key == key ? entry : NullEntry;
    }

    void TranspositionTable::write(u64 key, Move& best_move, int score, uint8_t depth, EntryType flag) {
        int index = get_index(key);
        Entry& entry = data[index];

        if (entry.key != key || entry.depth < depth) {
            data[index] = {key, best_move, score, depth, flag};
        }
    }

    int TranspositionTable::hashfull() {
        
        int filled = 0;
        for (int i = 0; i < 1000; ++i) {
            if (data[i].key != 0) ++filled;
        }
        return filled;
    }

    void TranspositionTable::clear() {
        std::memset(data, 0, sizeof(data));
    }

}