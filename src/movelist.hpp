#pragma once

#include "move.hpp"
#include "position.hpp"

namespace Crystall {
    class MoveList {
        private:

        Move moves[256];
        int scores[256];
        int size_ = 0;
        const Position& pos;

        public:
        inline MoveList(const Position& pos) : size_(pos.generate_pseudo_legal_moves(moves)), pos(pos) {}

        inline Move operator[](int i) { return moves[i]; }
        inline const Move operator[](int i) const { return moves[i]; }
        inline int size() { return size_; }

        void calculate_scores(const Move& special_move);
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
            std::swap(moves[i], moves[highest_score_index]);

            return true;
        }
        
    };
}