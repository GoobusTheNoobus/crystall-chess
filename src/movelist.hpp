// Crystall is a hobby UCI chess engine written in C++
// Developed by GoobusTheNoobus

#pragma once

#include "move.hpp"
#include "position.hpp"
#include "movegen.hpp"

namespace Crystall {

    namespace TranspositionTable { struct Bucket; }
    
    class MoveList {
        private:

        u16 moves[256];
        int scores[256];
        int size_ = 0;
        const Position& pos;

        public:
        inline MoveList(const Position& pos) : size_(MoveGen::generate_pseudo_legal_moves(pos, moves)), pos(pos) {}

        inline u16 operator[](int i) { return moves[i]; }
        inline const u16 operator[](int i) const { return moves[i]; }
        inline int size() { return size_; }

        void calculate_scores(const u16 special_move);
        void calculate_scores(const TranspositionTable::Bucket& bucket);
        void calculate_scores();
        inline bool next(int i) {
            
            if (i >= size_) return false;

            int highest_score_index = i;
            int highest_score = scores[i];

            for (int j = i; j < size_; ++j) {
                if (scores[j] > highest_score) {
                    highest_score = scores[j];
                    highest_score_index = j;
                }
            }

            std::swap(moves[i], moves[highest_score_index]);
            std::swap(scores[i], scores[highest_score_index]);

            return true;
        }
        
    };
}