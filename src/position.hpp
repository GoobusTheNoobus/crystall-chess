#pragma once

#include "types.hpp"
#include <string>

namespace Crystall 
{

    // position representation

    struct GameState 
    {
        Square en_passant_square = Square::None;
        int castling_rights = 0;
        int rule50_clock = 0;
    };

    class Position 
    {

        // members
        private:

        // board representation
        Piece board[SQUARE_NB];
        Bitboard piece_bitboards[PIECE_NB];
        Bitboard color_bitboards[COLOR_NB];
        Bitboard occupancy = 0;

        // game states
        Color side_to_move;
        GameState state;

        // public functions & constructors
        public:

        Position();
        
        void parse_fen(const std::string& fen);
        std::string to_string() const;

        // lookups
        inline Piece get_piece_on(Square s) const { return board[int(s)]; }
        inline Bitboard get_bitboard(Color c) const { return color_bitboards[int(c)]; }
        inline Bitboard get_bitboard(Piece p) const { return piece_bitboards[int(p)]; }
        inline Bitboard get_bitboard(PieceType pt, Color c) const { return piece_bitboards[int(make_piece(pt, c))]; }

        inline Color get_side_to_move() const { return side_to_move; }
        inline Square get_en_passant() const { return state.en_passant_square; }
        inline bool has_castling_right(int mask) const { return state.castling_rights & mask; }
        inline bool is_rule_50() const { return state.rule50_clock >= 100; }

        // helper functions
        private:

        void clear();
        void clear_square(Square square);
        void place_piece(Square square, Piece piece);

    };

    std::ostream& operator<<(std::ostream& os, const Position& pos);
}



