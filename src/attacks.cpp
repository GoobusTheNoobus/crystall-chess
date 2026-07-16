#include "attacks.hpp"
#include "bitboards.hpp"

namespace Crystall::Attack {

    bool is_attacked(const Position& pos, Square square, Color by) {

        if (Bitboards::pawn_attacks(square, opposite(by)) & pos.get_bitboard(Pawn, by)) return true;

        if (Bitboards::knight_attacks(square) & pos.get_bitboard(Knight, by)) return true;

        if (Bitboards::bishop_attack(square, pos.get_bitboard(White) | pos.get_bitboard(Black)) & (pos.get_bitboard(Bishop, by) | pos.get_bitboard(Queen, by))) return true;

        if (Bitboards::rook_attack(square, pos.get_bitboard(White) | pos.get_bitboard(Black)) & (pos.get_bitboard(Rook, by) | pos.get_bitboard(Queen, by))) return true;

        if (Bitboards::king_attacks(square) & pos.get_bitboard(King, by)) return true;
        
        return false;
    }
}
