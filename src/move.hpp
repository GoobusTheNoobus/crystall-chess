#pragma once

#include "types.hpp"

namespace Crystall {

    struct Move {
        enum Type {
            Normal,
            DoublePawnPush, 
            Castling,
            EnPassant,
            PromoQ,
            PromoR,
            PromoB,
            PromoN,
            None
        };

        private:
        Square from_;
        Square dest_;
        Type flag_;

        public:
        // Move lookups
        inline Square from() const { return from_; }
        inline Square dest() const { return dest_; }
        inline Type flag() const { return flag_; }

        inline Move(Square from, Square dest, Type flag) : from_(from), dest_(dest), flag_(flag) {}
        inline constexpr Move() : from_(NoSquare), dest_(NoSquare), flag_(None) {}

        inline bool is_valid() const {
            return from_ != NoSquare && dest_ != NoSquare && flag_ != None;
        }

        inline bool operator==(const Move& other) const {
            return from_ == other.from_ && dest_ == other.dest_ && flag_ == other.flag_;
        }

        constexpr static char PromoCharacters[4] = {'q', 'r', 'b', 'n'};

        inline std::string to_string() const {
            if (!is_valid()) return "0000";

            std::string from_str = square_to_string(from_);
            std::string dest_str = square_to_string(dest_);

            if (flag_ >= PromoQ) {
                return from_str + dest_str + PromoCharacters[flag_ - PromoQ];
            }

            return from_str + dest_str;
        }

        static const Move NullMove;
    };
}