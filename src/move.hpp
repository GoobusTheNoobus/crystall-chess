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

        inline Square from() const { return from_; }
        inline Square dest() const { return dest_; }
        inline Type flag() const { return flag_; }

        inline Move(Square from, Square dest, Type flag) : from_(from), dest_(dest), flag_(flag) {}
        inline constexpr Move() : from_(NoSquare), dest_(NoSquare), flag_(None) {}

        bool is_valid() const;
        bool operator==(const Move&) const;
        std::string to_string() const;

        static const Move NullMove;

    };

    inline std::ostream& operator<<(std::ostream& os, const Move& move) {
        os << move.to_string();
        return os;
    }
}