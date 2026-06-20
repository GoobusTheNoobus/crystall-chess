#pragma once

#include <string>
#include <cstdint>

namespace Crystall {

    using u64 = uint64_t;

    // enums & constants

    constexpr int FILE_NB = 8, 
                  RANK_NB = 8, 
                  SQUARE_NB = FILE_NB * RANK_NB;
    enum Square : uint8_t {
        A1, B1, C1, D1, E1, F1, G1, H1,
        A2, B2, C2, D2, E2, F2, G2, H2,
        A3, B3, C3, D3, E3, F3, G3, H3,
        A4, B4, C4, D4, E4, F4, G4, H4,
        A5, B5, C5, D5, E5, F5, G5, H5,
        A6, B6, C6, D6, E6, F6, G6, H6,
        A8, B8, C8, D8, E8, F8, G8, H8,
        A7, B7, C7, D7, E7, F7, G7, H7,

        NO_SQUARE
    };

    constexpr int COLOR_NB = 2, 
                  PIECETYPE_NB = 6, 
                  PIECE_NB = COLOR_NB * PIECETYPE_NB;
    enum Color : uint8_t { WHITE, BLACK };
    enum PieceType : uint8_t { PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING };
    enum Piece : uint8_t {
        WHITE_PAWN, WHITE_KNIGHT, WHITE_BISHOP, WHITE_ROOK, WHITE_QUEEN, WHITE_KING,
        BLACK_PAWN, BLACK_KNIGHT, BLACK_BISHOP, BLACK_ROOK, BLACK_QUEEN, BLACK_KING,
        NO_PIECE
    };

    // helpers

    inline Piece make_piece(PieceType pt, Color c) { return Piece(int(c) * 6 + int(pt)); }
    inline PieceType type_of(Piece p) { return PieceType(int(p) % 6); }
    inline Color color_of(Piece p) { return Color(int(p) / 6); }
    inline Color opposite(Color c) { return Color(c ^ 1); }

    inline Square make_square(int r, int f) { return Square(r * 8 + f); }
    inline int file_of(Square s) { return int(s) % 8; }
    inline int rank_of(Square s) { return int(s) / 8; }

    inline Square make_square(std::string str) {

        char rc = str[1];
        char fc = str[0];

        int rank = rc - '1';
        int file = fc - 'a';

        if (unsigned(rank) > 7 || unsigned(file) > 7) return NO_SQUARE;

        return make_square(rank, file);
    }

    inline std::string square_to_string(Square square) {
        char rc = rank_of(square) + '1';
        char fc = file_of(square) + 'a';

        return {fc, rc};
    }

    enum Castling : int {
        CASTLING_WK = 1, CASTLING_WQ = 2, CASTLING_BK = 4, CASTLING_BQ = 8
    };
}