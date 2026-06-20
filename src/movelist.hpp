#pragma once

#include "move.hpp"
#include "position.hpp"

namespace Crystall {
    class MoveList {
        private:

        Move moves[256];
        int size_ = 0;

        public:
        inline MoveList(const Position& pos) : size_(pos.generate_pseudo_legal_moves(moves)) {}

        inline Move operator[](int i) { return moves[i]; }
        inline const Move operator[](int i) const { return moves[i]; }

        inline int size() { return size_; }

        inline std::string to_string() const {
            std::string str = "[";
            for (int i = 0; i < size_ - 1; ++i) {
                str += moves[i].to_string() + ", ";
            }
            str += moves[size_ - 1].to_string();

            return str + ']' + "\n";
        }
    };
}