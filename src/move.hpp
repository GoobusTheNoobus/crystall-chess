#pragma once

#include "types.hpp"

namespace Crystall {
    struct Move {
        enum Type {
            NORMAL,
            // double pawn pushes result in an active en passant square,
            // so it's more efficient to have a seperate move flag
            DOUBLE_PAWN_PUSH, 
            CASTLING,
            EN_PASSANT,
            PROMO_Q,
            PROMO_R,
            PROMO_B,
            PROMO_N,
            NONE
        };

        private:
        Square from_;
        Square dest_;
        Type flag_;

        public:
        // lookups
        inline Square from() { return from_; }
        inline Square dest() { return dest_; }
        inline Type flag() { return flag_; }

        inline Move(Square from, Square dest, Type flag) : from_(from), dest_(dest), flag_(flag) {}
        inline Move() : from_(NO_SQUARE), dest_(NO_SQUARE), flag_(NONE) {}

        constexpr static char PromoCharacters[4] = {'q', 'r', 'b', 'n'};

        inline std::string to_string() const {
            std::string from_str = square_to_string(from_);
            std::string dest_str = square_to_string(dest_);

            if (flag_ >= PROMO_Q) {
                return from_str + dest_str + PromoCharacters[flag_ - PROMO_Q];
            }

            return from_str + dest_str;
        }
    };
}