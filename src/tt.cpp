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

        for (int i = 0; i < BucketSize; i++) {
            if (bucket.entries[i].key == key) {

                if (depth >= bucket.entries[i].depth)
                    bucket.entries[i] = { key, score, best_move, depth, flag };

                return;
            }
        }

        for (int i = 0; i < BucketSize; i++) {
            if (bucket.entries[i].key == 0) {
                bucket.entries[i] = { key, score, best_move, depth, flag };
                return;
            }
        }

        Entry* replace = &bucket.entries[0];

        for (int i = 1; i < BucketSize; i++) {
            if (bucket.entries[i].depth < replace->depth)
                replace = &bucket.entries[i];
        }

        if (depth >= replace->depth)
            *replace = { key, score, best_move, depth, flag };
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