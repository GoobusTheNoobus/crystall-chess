// Crystall is a hobby UCI chess engine written in C++
// Developed by GoobusTheNoobus

#include "attacks.hpp"
#include "bitboards.hpp"

namespace Crystall::Attack {

    // Detects whether a square is attacked by a given side on the position
    bool is_attacked(const Position& pos, Square square, Color by) {

        // Pawns
        if (Bitboards::pawn_attacks(square, opposite(by)) & pos.get_bitboard(Pawn, by)) 
            return true;

        // Knights
        if (Bitboards::knight_attacks(square) & pos.get_bitboard(Knight, by)) 
            return true;

        // Bishops + Queens
        if (Bitboards::bishop_attack(square, pos.get_bitboard(White) | pos.get_bitboard(Black)) & (pos.get_bitboard(Bishop, by) | pos.get_bitboard(Queen, by))) 
            return true;

        // Rooks + Queens
        if (Bitboards::rook_attack(square, pos.get_bitboard(White) | pos.get_bitboard(Black)) & (pos.get_bitboard(Rook, by) | pos.get_bitboard(Queen, by))) 
            return true;

        // kings
        if (Bitboards::king_attacks(square) & pos.get_bitboard(King, by)) 
            return true;
        
        // No attackers
        return false;
    }
}
