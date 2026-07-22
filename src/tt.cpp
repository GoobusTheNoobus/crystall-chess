// Crystall is a hobby UCI chess engine written in C++
// Developed by GoobusTheNoobus

#include "tt.hpp"

#include <cstring>

namespace Crystall::TranspositionTable {

    static inline int get_index(u64 key) { return key % BucketCount; }

    const Bucket& probe(u64 key) {
        int index = get_index(key);

        return data[index];
    }

    void write(u64 key, u16 best_move, int score, u8 depth, EntryType flag) {
        int index = get_index(key);
        Bucket& bucket = data[index];

        Entry* entry_getting_replaced = nullptr;
        int lowest_depth = 900;
        for (int i = 0; i < BucketSize; ++i) {
            Entry* current = &bucket.entries[i];
            if (lowest_depth > current->depth && (current->key == key || current->key == 0) && depth > current->depth) {
                entry_getting_replaced = current;
                lowest_depth = current->depth;
            }
        }

        // None of the entries were suitable for reaplacement
        if (entry_getting_replaced == nullptr) {
            return;
        }

        // We write to the lowest depth entry, since it is likely the most worthless
        *entry_getting_replaced = {key, score, best_move, depth, flag};
    }

    int hashfull() {
        
        int filled = 0;
        for (int i = 0; i < 1000; ++i) {
            filled += (data[i].entries[0].key + data[i].entries[1].key + data[i].entries[2].key + data[i].entries[3].key) != 0;
        }
        return filled;
    }

    void clear() {
        std::memset(data, 0, sizeof(data));
    }

}