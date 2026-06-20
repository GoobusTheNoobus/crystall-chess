#include "position.hpp"
#include "move.hpp"
#include "bit.hpp"

#include <sstream>
#include <iostream>
#include <charconv>

namespace Crystall {
    constexpr char PieceCharacters[] = "PNBRQKpnbrqk";
    constexpr char StartingPositionFen[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    std::ostream& operator<<(std::ostream& os, const Position& pos) {
        os << pos.to_string();
        return os;
    }

    // Default constructor sets up starting position
    Position::Position() {
        parse_fen(StartingPositionFen);
    }

    // Parses a FEN position. 
    // An invalid FEN leads to undefined behavior
    void Position::parse_fen(const std::string& fen) {
        clear();

        std::istringstream iss(fen);

        std::string fen_board_part;
        if (!(iss >> fen_board_part)) return;

        int r = 7, f = 0;
        for (char c : fen_board_part) {
            if (c == '/') {
                r -= 1;
                f = 0;
                continue;
            }

            if (std::isdigit(c)) {
                f += c - '0';
                continue;
            }

            Piece p = NO_PIECE;

            for (int i = 0; i < 13; ++i) {
                if (PieceCharacters[i] == c) 
                    p = Piece(i);
            }

            place_piece(make_square(r, f), p);
            ++f;
        }

        std::string fen_side_part;
        if (!(iss >> fen_side_part)) return;

        side_to_move = fen_side_part == "w" ? WHITE : BLACK;

        std::string fen_castling_part;
        if (!(iss >> fen_castling_part)) return;

        for (char c : fen_castling_part) {
            switch (c) {
                case 'K': state.castling_rights |= CASTLING_WK; break;
                case 'Q': state.castling_rights |= CASTLING_WQ; break;
                case 'k': state.castling_rights |= CASTLING_BK; break;
                case 'q': state.castling_rights |= CASTLING_BQ; break;
            }
        }

        std::string fen_ep_part;
        if (!(iss >> fen_ep_part)) return;

        state.en_passant_square = make_square(fen_ep_part);

        std::string fen_rule50_part;
        if (!(iss >> fen_rule50_part)) return;

        std::from_chars(
            fen_rule50_part.data(),
            fen_rule50_part.data() + fen_rule50_part.size(),
            state.rule50_clock
        );
    }

    std::string Position::to_string() const {
        std::ostringstream oss;

        oss << '\n';
        for (int r = 7; r >= 0; --r) {
            
            oss << "  +---+---+---+---+---+---+---+---+\n";
            oss << (r + 1) << ' ';
            for (int f = 0; f < 8; ++f) {
                oss << "| ";

                Piece piece = get_piece_on(make_square(r, f));

                if (piece != NO_PIECE) oss << PieceCharacters[int(piece)];
                else oss << ' ';

                oss << ' ';
            }
            oss << "|\n";
        }
        oss << "  +---+---+---+---+---+---+---+---+\n";
        oss << "    a   b   c   d   e   f   g   h  \n\n";

        return oss.str();
    }

    void Position::clear() {
        for (int i = 0; i < SQUARE_NB; ++i) board[i] = NO_PIECE;      
        for (int i = 0; i < PIECE_NB; ++i) piece_bitboards[i] = 0;
        for (int i = 0; i < COLOR_NB; ++i) color_bitboards[i] = 0;
        occupancy = 0;
        
        side_to_move = WHITE;
        state.castling_rights = 0;
        state.en_passant_square = NO_SQUARE;
        state.rule50_clock = 0;
    }

    void Position::clear_square(Square square) {
        if (get_piece_on(square) == NO_PIECE) return;

        Piece piece_already_there = get_piece_on(square);
        Color color = color_of(piece_already_there);

        u64 mask = ~(1ULL << int(square));

        board[int(square)] = NO_PIECE;

        piece_bitboards[int(piece_already_there)] &= mask;
        color_bitboards[int(color)] &= mask;
        occupancy &= mask;
    }

    // This assumes that the square is empty
    // If you are not sure that the square is empty, 
    // clear it first in case
    void Position::place_piece(Square square, Piece piece) {
        if (piece == NO_PIECE) {
            clear_square(square);
            return;
        }

        Color color = color_of(piece);

        u64 mask = 1ULL << int(square);

        board[int(square)] = piece;
        piece_bitboards[int(piece)] |= mask;
        color_bitboards[int(color)] |= mask;
        occupancy |= mask;
    }

    bool Position::is_attacked(Square square, Color by) const {

        if (Bitboards::pawn_attacks(square, opposite(by)) & get_bitboard(PAWN, by)) return true; 
        if (Bitboards::knight_attacks(square) & get_bitboard(KNIGHT, by)) return true;
        if (Bitboards::bishop_attack(square, occupancy) & (get_bitboard(BISHOP, by) | get_bitboard(QUEEN, by))) return true;
        if (Bitboards::rook_attack(square, occupancy) & (get_bitboard(ROOK, by) | get_bitboard(QUEEN, by))) return true;
        if (Bitboards::king_attacks(square) & (get_bitboard(KING, by))) return true;

        return false;
    }

    bool Position::is_in_check(Color color) const {
        return is_attacked(Square(ctz(get_bitboard(KING, color))), opposite(color));
    }

    bool Position::is_in_check() const {
        return is_in_check(side_to_move);
    }

    namespace {
        // helpers for movegen
        void add(int& i, Move* arr, Move m) {
            arr[i++] = m;
        }

        void extract_pawn(int& i, Move* arr, u64 bb, int offset, Move::Type type) {
            while (bb) {
                int lsb = poplsb(bb);
                add(i, arr, Move(Square(lsb - offset), Square(lsb), type));
            }
        }

        void extract_pawn_promo(int& i, Move* arr, u64 bb, int offset) {
            while (bb) {
                int lsb = poplsb(bb);
                add(i, arr, Move(Square(lsb - offset), Square(lsb), Move::PROMO_Q));
                add(i, arr, Move(Square(lsb - offset), Square(lsb), Move::PROMO_R));
                add(i, arr, Move(Square(lsb - offset), Square(lsb), Move::PROMO_B));
                add(i, arr, Move(Square(lsb - offset), Square(lsb), Move::PROMO_N));
            }
        }
    }

    // takes in an array as an argument and returns the number of moves there are
    int Position::generate_pseudo_legal_moves(Move arr[]) const {
        int size = 0;
        Color us = side_to_move;
        Color them = opposite(us);
        bool is_white = us == WHITE;

        // Pawn moves
        u64 pawns = get_bitboard(PAWN, us);
        u64 occ   = occupancy;
        u64 enemy = get_bitboard(them);

        u64 rank3_from_bottom = is_white ? Bitboards::RankBB[2] : Bitboards::RankBB[5];
        u64 rank8_from_bottom = is_white ? Bitboards::RankBB[7] : Bitboards::RankBB[0];

        int single_push_offset = is_white ? 8 : -8;
        int double_push_offset = single_push_offset * 2;
        int left_capture_offset = is_white ? 7 : -9;
        int right_capture_offset = is_white ? 9 : -7;

        // compute bitboards
        u64 single_push_bb = (is_white ? pawns << 8 : pawns >> 8) & ~occ;
        u64 double_push_bb = (is_white ? (single_push_bb & rank3_from_bottom) << 8 : (single_push_bb & rank3_from_bottom) >> 8) & ~occ;
        u64 left_capture_bb = (is_white ? (pawns & ~Bitboards::FileBB[0]) << 7 : (pawns & ~Bitboards::FileBB[7]) >> 9) & enemy;
        u64 right_capture_bb = (is_white ? (pawns & ~Bitboards::FileBB[7]) << 9 : (pawns & ~Bitboards::FileBB[0]) >> 7) & enemy;

        u64 single_push_promo_bb = single_push_bb & rank8_from_bottom;
        u64 single_push_normal_bb = single_push_bb & ~rank8_from_bottom;
        u64 left_promo_bb = left_capture_bb & rank8_from_bottom;
        u64 left_normal_bb = left_capture_bb & ~rank8_from_bottom;
        u64 right_promo_bb = right_capture_bb & rank8_from_bottom;
        u64 right_normal_bb = right_capture_bb & ~rank8_from_bottom;

        extract_pawn(size, arr, single_push_normal_bb, single_push_offset, Move::NORMAL);
        extract_pawn(size, arr, double_push_bb, double_push_offset, Move::DOUBLE_PAWN_PUSH);
        extract_pawn(size, arr, right_normal_bb, right_capture_offset, Move::NORMAL);
        extract_pawn(size, arr, left_normal_bb, left_capture_offset, Move::NORMAL);

        extract_pawn_promo(size, arr, single_push_promo_bb, single_push_offset);
        extract_pawn_promo(size, arr, left_promo_bb, left_capture_offset);
        extract_pawn_promo(size, arr, right_promo_bb, right_capture_offset);

        Square ep = get_en_passant();

        if (ep != NO_SQUARE) {
            u64 ep_pawns = pawns & Bitboards::pawn_attacks(ep, them);
            while (ep_pawns) {
                int lsb = poplsb(ep_pawns);
                add(size, arr, Move(Square(lsb), ep, Move::EN_PASSANT));
            }
        }

        // Knights
        u64 knights = get_bitboard(KNIGHT, us);

        while (knights) {
            Square from = Square(poplsb(knights));

            u64 attacks = Bitboards::knight_attacks(from) & ~get_bitboard(us);

            while (attacks) {
                Square to = Square(poplsb(attacks));
                add(size, arr, Move(from, to, Move::NORMAL));
            }
        }

        // Bishops
        u64 bishops = get_bitboard(BISHOP, us);

        while (bishops) {
            Square from = Square(poplsb(bishops));

            u64 attacks = Bitboards::bishop_attack(from, occ) & ~get_bitboard(us);

            while (attacks) {
                Square to = Square(poplsb(attacks));
                add(size, arr, Move(from, to, Move::NORMAL));
            }
        }

        // Rooks
        u64 rooks = get_bitboard(ROOK, us);

        while (rooks) {
            Square from = Square(poplsb(rooks));

            u64 attacks = Bitboards::rook_attack(from, occ) & ~get_bitboard(us);

            while (attacks) {
                Square to = Square(poplsb(attacks));
                add(size, arr, Move(from, to, Move::NORMAL));
            }
        }

        // Queens
        u64 queens = get_bitboard(QUEEN, us);

        while (queens) {
            Square from = Square(poplsb(queens));

            u64 attacks = Bitboards::queen_attack(from, occ) & ~get_bitboard(us);

            while (attacks) {
                Square to = Square(poplsb(attacks));
                add(size, arr, Move(from, to, Move::NORMAL));
            }
        }

        // King
        u64 king = get_bitboard(KING, us);

        if (king) {
            Square from = Square(poplsb(king));

            u64 attacks = Bitboards::king_attacks(from) & ~get_bitboard(us);

            while (attacks) {
                Square to = Square(poplsb(attacks));
                add(size, arr, Move(from, to, Move::NORMAL));
            }
        } else {
            std::cerr << "NO KING ON BOARD" << std::endl;
            exit(-1);
        }

        constexpr static u64 WK_CASTLE_EMPTY = Bitboards::SquareBB[F1] | Bitboards::SquareBB[G1];
        constexpr static u64 WQ_CASTLE_EMPTY = Bitboards::SquareBB[D1] | Bitboards::SquareBB[C1] | Bitboards::SquareBB[B1];
        constexpr static u64 BK_CASTLE_EMPTY = Bitboards::SquareBB[F8] | Bitboards::SquareBB[G8];
        constexpr static u64 BQ_CASTLE_EMPTY = Bitboards::SquareBB[D8] | Bitboards::SquareBB[C8] | Bitboards::SquareBB[B8];

        // castling
        if (is_white && !is_attacked(E1, them)) {
            // kingside
            if (has_castling_right(CASTLING_WK) && 
                !(occ & WK_CASTLE_EMPTY) && 
                !is_attacked(F1, them) && !is_attacked(G1, them)) 

                add(size, arr, Move(E1, G1, Move::CASTLING));

            // queenside
            if (has_castling_right(CASTLING_WQ) && 
                !(occ & WQ_CASTLE_EMPTY) && 
                !is_attacked(D1, them) && !is_attacked(C1, them)) 
                
                add(size, arr, Move(E1, C1, Move::CASTLING));

        } else if (!is_white && !is_attacked(E8, them)) {
            // kingside
            if (has_castling_right(CASTLING_BK) && 
                !(occ & BK_CASTLE_EMPTY) && 
                !is_attacked(F8, them) && !is_attacked(G8, them)) 
                
                add(size, arr, Move(E8, G8, Move::CASTLING));

            // queenside
            if (has_castling_right(CASTLING_BQ) && 
                !(occ & BQ_CASTLE_EMPTY) && 
                !is_attacked(D8, them) && !is_attacked(C8, them)) 
                
                add(size, arr, Move(E8, C8, Move::CASTLING));
        }

        return size;
    }
}