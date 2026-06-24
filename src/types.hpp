#pragma once

#include <string>
#include <cstdint>
#include <cmath>

namespace Crystall {

    using u64 = uint64_t;

    constexpr int FileNB = 8, RankNB = 8, SquareNB = FileNB * RankNB;
    enum Square : uint8_t {
        A1, B1, C1, D1, E1, F1, G1, H1,
        A2, B2, C2, D2, E2, F2, G2, H2,
        A3, B3, C3, D3, E3, F3, G3, H3,
        A4, B4, C4, D4, E4, F4, G4, H4,
        A5, B5, C5, D5, E5, F5, G5, H5,
        A6, B6, C6, D6, E6, F6, G6, H6,
        A7, B7, C7, D7, E7, F7, G7, H7,
        A8, B8, C8, D8, E8, F8, G8, H8,

        NoSquare
    };

    constexpr int ColorNB = 2, PieceTypeNB = 6, PieceNB = ColorNB * PieceTypeNB;
    enum Color : uint8_t { White, Black };
    enum PieceType : uint8_t { Pawn, Knight, Bishop, Rook, Queen, King };
    enum Piece : uint8_t {
        WhitePawn, WhiteKnight, WhiteBishop, WhiteRook, WhiteQueen, WhiteKing,
        BlackPawn, BlackKnight, BlackBishop, BlackRook, BlackQueen, BlackKing,
        NoPiece
    };

    // helpers

    inline Piece make_piece(PieceType pt, Color c) { return Piece(c * 6 + pt); }
    inline PieceType type_of(Piece p) { return PieceType(p % 6); }
    inline Color color_of(Piece p) { return Color(p / 6); }
    inline Color opposite(Color c) { return Color(c ^ 1); }

    inline Square make_square(int r, int f) { return Square(r * 8 + f); }
    inline int file_of(Square s) { return s % 8; }
    inline int rank_of(Square s) { return s / 8; }

    inline Square make_square(std::string str) {

        char rc = str[1];
        char fc = str[0];

        int rank = rc - '1';
        int file = fc - 'a';

        if (unsigned(rank) > 7 || unsigned(file) > 7) return NoSquare;

        return make_square(rank, file);
    }

    inline std::string square_to_string(Square square) {
        char rc = rank_of(square) + '1';
        char fc = file_of(square) + 'a';

        return {fc, rc};
    }

    constexpr int CastlingWK = 1, CastlingWQ = 2, CastlingBK = 4, CastlingBQ = 8;
    constexpr int DrawScore = 0, MaxCentipawn = 10000, MateScore = 11000, Infinity = 11001, Timeout = 11002;

    inline std::string score_string(int score) {
        if (std::abs(score) <= MaxCentipawn) return "cp " + std::to_string(score);

        int mate_dist = MateScore - std::abs(score);
        
        mate_dist = score > 0 ? mate_dist : -mate_dist;
        mate_dist = (int)std::ceil(mate_dist / 2.0);

        return "mate " + std::to_string(mate_dist);
    }
}