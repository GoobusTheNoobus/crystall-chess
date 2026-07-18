// Crystall is a hobby UCI chess engine written in C++
// Developed by GoobusTheNoobus

#include "movegen.hpp"
#include "attacks.hpp"
#include "bitboards.hpp"

#include <iostream>

namespace Crystall::MoveGen {
    namespace {
        void add(int& i, u16* arr, u16 m) {
            arr[i++] = m;
        }

        void extract_pawn(int& i, u16* arr, u64 bb, int offset, Move::Type type) {
            while (bb) {
                int lsb = poplsb(bb);
                add(i, arr, Move::create(Square(lsb - offset), Square(lsb), type));
            }
        }

        void extract_pawn_promo(int& i, u16* arr, u64 bb, int offset) {
            while (bb) {
                int lsb = poplsb(bb);
                add(i, arr, Move::create(Square(lsb - offset), Square(lsb), Move::PromoQ));
                add(i, arr, Move::create(Square(lsb - offset), Square(lsb), Move::PromoR));
                add(i, arr, Move::create(Square(lsb - offset), Square(lsb), Move::PromoB));
                add(i, arr, Move::create(Square(lsb - offset), Square(lsb), Move::PromoN));
            }
        }
    }

    int generate_pseudo_legal_moves(const Position& pos, u16 moves[]) {
        int size = 0;
        Color us = pos.get_side_to_move();
        Color them = opposite(us);
        bool is_white = us == White;

        u64 pawns = pos.get_bitboard(Pawn, us);
        u64 occ = pos.get_bitboard(White) | pos.get_bitboard(Black);
        u64 enemy = pos.get_bitboard(them);

        u64 rank3_from_bottom = is_white ? Bitboards::RankBB[2] : Bitboards::RankBB[5];
        u64 rank8_from_bottom = is_white ? Bitboards::RankBB[7] : Bitboards::RankBB[0];

        int single_push_offset = is_white ? 8 : -8;
        int double_push_offset = single_push_offset * 2;
        int left_capture_offset = is_white ? 7 : -9;
        int right_capture_offset = is_white ? 9 : -7;

        u64 single_push_bb = (is_white ? pawns << 8 : pawns >> 8) & ~occ;
        u64 double_push_bb = (is_white ? (single_push_bb & rank3_from_bottom) << 8 : (single_push_bb & rank3_from_bottom) >> 8) & ~occ;
        u64 left_capture_bb = (is_white ? (pawns & ~Bitboards::FileBB[0]) << 7 : (pawns & ~Bitboards::FileBB[0]) >> 9) & enemy;
        u64 right_capture_bb = (is_white ? (pawns & ~Bitboards::FileBB[7]) << 9 : (pawns & ~Bitboards::FileBB[7]) >> 7) & enemy;

        u64 single_push_promo_bb = single_push_bb & rank8_from_bottom;
        u64 single_push_normal_bb = single_push_bb & ~rank8_from_bottom;
        u64 left_promo_bb = left_capture_bb & rank8_from_bottom;
        u64 left_normal_bb = left_capture_bb & ~rank8_from_bottom;
        u64 right_promo_bb = right_capture_bb & rank8_from_bottom;
        u64 right_normal_bb = right_capture_bb & ~rank8_from_bottom;

        extract_pawn(size, moves, single_push_normal_bb, single_push_offset, Move::Normal);
        extract_pawn(size, moves, double_push_bb, double_push_offset, Move::DoublePawnPush);
        extract_pawn(size, moves, right_normal_bb, right_capture_offset, Move::Normal);
        extract_pawn(size, moves, left_normal_bb, left_capture_offset, Move::Normal);

        extract_pawn_promo(size, moves, single_push_promo_bb, single_push_offset);
        extract_pawn_promo(size, moves, left_promo_bb, left_capture_offset);
        extract_pawn_promo(size, moves, right_promo_bb, right_capture_offset);

        Square ep = pos.get_en_passant();
        if (ep != NoSquare) {
            u64 ep_pawns = pawns & Bitboards::pawn_attacks(ep, opposite(us));
            while (ep_pawns) {
                int lsb = poplsb(ep_pawns);
                add(size, moves, Move::create(Square(lsb), ep, Move::EnPassant));
            }
        }

        u64 knights = pos.get_bitboard(Knight, us);
        while (knights) {
            Square from = Square(poplsb(knights));
            u64 attacks = Bitboards::knight_attacks(from) & ~pos.get_bitboard(us);
            while (attacks) {
                Square to = Square(poplsb(attacks));
                add(size, moves, Move::create(from, to, Move::Normal));
            }
        }

        u64 bishops = pos.get_bitboard(Bishop, us);
        while (bishops) {
            Square from = Square(poplsb(bishops));
            u64 attacks = Bitboards::bishop_attack(from, occ) & ~pos.get_bitboard(us);
            while (attacks) {
                Square to = Square(poplsb(attacks));
                add(size, moves, Move::create(from, to, Move::Normal));
            }
        }

        u64 rooks = pos.get_bitboard(Rook, us);
        while (rooks) {
            Square from = Square(poplsb(rooks));
            u64 attacks = Bitboards::rook_attack(from, occ) & ~pos.get_bitboard(us);
            while (attacks) {
                Square to = Square(poplsb(attacks));
                add(size, moves, Move::create(from, to, Move::Normal));
            }
        }

        u64 queens = pos.get_bitboard(Queen, us);
        while (queens) {
            Square from = Square(poplsb(queens));
            u64 attacks = Bitboards::queen_attack(from, occ) & ~pos.get_bitboard(us);
            while (attacks) {
                Square to = Square(poplsb(attacks));
                add(size, moves, Move::create(from, to, Move::Normal));
            }
        }

        u64 king = pos.get_bitboard(King, us);
        if (king) {
            Square from = Square(poplsb(king));
            u64 attacks = Bitboards::king_attacks(from) & ~pos.get_bitboard(us);
            while (attacks) {
                Square to = Square(poplsb(attacks));
                add(size, moves, Move::create(from, to, Move::Normal));
            }
        }

        constexpr u64 WKCastleEmpty = Bitboards::SquareBB[F1] | Bitboards::SquareBB[G1];
        constexpr u64 WQCastleEmpty = Bitboards::SquareBB[D1] | Bitboards::SquareBB[C1] | Bitboards::SquareBB[B1];
        constexpr u64 BKCastleEmpty = Bitboards::SquareBB[F8] | Bitboards::SquareBB[G8];
        constexpr u64 BQCastleEmpty = Bitboards::SquareBB[D8] | Bitboards::SquareBB[C8] | Bitboards::SquareBB[B8];

        if (is_white && !pos.is_attacked(E1, them)) {
            if (pos.has_castling_right(CastlingWK) && !(occ & WKCastleEmpty) && !pos.is_attacked(F1, them) && !pos.is_attacked(G1, them)) {
                add(size, moves, Move::create(E1, G1, Move::Castling));
            }
            if (pos.has_castling_right(CastlingWQ) && !(occ & WQCastleEmpty) && !pos.is_attacked(D1, them) && !pos.is_attacked(C1, them)) {
                add(size, moves, Move::create(E1, C1, Move::Castling));
            }
        } else if (!is_white && !pos.is_attacked(E8, them)) {
            if (pos.has_castling_right(CastlingBK) && !(occ & BKCastleEmpty) && !pos.is_attacked(F8, them) && !pos.is_attacked(G8, them)) {
                add(size, moves, Move::create(E8, G8, Move::Castling));
            }
            if (pos.has_castling_right(CastlingBQ) && !(occ & BQCastleEmpty) && !pos.is_attacked(D8, them) && !pos.is_attacked(C8, them)) {
                add(size, moves, Move::create(E8, C8, Move::Castling));
            }
        }

        return size;
    }
}
