#include "chess/move/move.hpp"

namespace Crystall {

    const Move Move::NullMove{};

    bool Move::is_valid() const {
        return from_ != NoSquare && dest_ != NoSquare && flag_ != None;
    }

    bool Move::operator==(const Move& other) const {
        return from_ == other.from_ && dest_ == other.dest_ && flag_ == other.flag_;
    }

    constexpr static char PromoCharacters[4] = {'q', 'r', 'b', 'n'};

    std::string Move::to_string() const {

        if (!is_valid()) return "None";

        std::string from_str = square_to_string(from_);
        std::string dest_str = square_to_string(dest_);

        if (flag_ >= PromoQ) {
            return from_str + dest_str + PromoCharacters[flag_ - PromoQ];
        }

        return from_str + dest_str;
    }
}