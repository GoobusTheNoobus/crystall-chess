#pragma once

#include "types.hpp"
#include "move.hpp"
#include <string>

namespace Crystall {

    // position representation
    struct GameState {
        Square en_passant_square = NO_SQUARE;
        int castling_rights = 0;
        int rule50_clock = 0;
    };

    class Position {

        // members
        private:

        // board representation
        Piece board[SQUARE_NB];
        u64 piece_bitboards[PIECE_NB];
        u64 color_bitboards[COLOR_NB];
        u64 occupancy = 0;

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
        inline u64 get_bitboard(Color c) const { return color_bitboards[int(c)]; }
        inline u64 get_bitboard(Piece p) const { return piece_bitboards[int(p)]; }
        inline u64 get_bitboard(PieceType pt, Color c) const { return piece_bitboards[int(make_piece(pt, c))]; }

        inline Color get_side_to_move() const { return side_to_move; }
        inline Square get_en_passant() const { return state.en_passant_square; }
        inline bool has_castling_right(int mask) const { return state.castling_rights & mask; }
        inline bool is_rule_50() const { return state.rule50_clock >= 100; }

        // big boy functions 
        bool is_attacked(Square, Color by) const;
        bool is_in_check(Color) const;
        bool is_in_check() const;

        // bigger boy functions
        int generate_pseudo_legal_moves(Move[]) const;


        // helper functions
        private:

        void clear();
        void clear_square(Square square);
        void place_piece(Square square, Piece piece);

    };

    std::ostream& operator<<(std::ostream& os, const Position& pos);
}



