#pragma once

#include "types.hpp"
#include "move.hpp"
#include "eval.hpp"

#include <string>
#include <iostream>
#include <random>

namespace Crystall {

    namespace Zobrist {

        inline u64 PieceSquareKeys[PieceNB][SquareNB];
        inline u64 CastlingKeys[16];
        inline u64 EnPassantKeys[8];
        inline u64 SideKey;

        // fill all variables with random values
        inline void init() {
            std::mt19937_64 rng(67);

            for (int i = 0; i < PieceNB; ++i) {
                for (int j = 0; j < SquareNB; ++j) {
                    PieceSquareKeys[i][j] = rng();
                }
            }

            for (int i = 0; i < 16; ++i) {
                CastlingKeys[i] = rng();
            }

            for (int i = 0; i < 8; ++i) {
                EnPassantKeys[i] = rng();
            }

            SideKey = rng();

        }
    }

    constexpr char StartingPositionFen[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    // position representation
    struct GameState {
        Square en_passant_square = NoSquare;
        int castling_rights = 0;
        int rule50_clock = 0;
    };

    // a struct that represents the information needed to undo 
    // a move. should be stored in stack
    struct MoveUndoInfo {
        u64 key;
        Move move;
        int castling_rights;
        int rule50_clock;
        Square en_passant_square;
        Piece captured_piece;
    };

    class Position {

        // members
        private:

        // board representation
        Piece board[SquareNB];
        u64 piece_bitboards[PieceNB];
        u64 color_bitboards[ColorNB];
        u64 occupancy = 0;

        // game states
        Color side_to_move;
        GameState state;
        u64 hash = 0;

        // stack(s)
        MoveUndoInfo move_undo_stack[1024];
        int ply = 0;

        Evaluation::TaperedScore psqt_scores;

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
        inline u64 get_key() const { return hash; }

        // big boy functions 
        bool is_attacked(Square, Color by) const;
        bool is_in_check(Color) const;
        bool is_in_check() const;

        // bigger boy functions
        int generate_pseudo_legal_moves(Move[]) const;
        void make_move(const Move&);
        void make_move(const std::string&);
        bool attempt_move(const Move&);
        void undo_move();

        int evaluate() const;
        bool is_repetition() const;

        // helper functions
        private:

        void clear();
        void clear_square(Square square);
        void place_piece(Square square, Piece piece);

        void push_move_stacks(u64 key, Move, int castling_rights, int rule50_clock, Square en_passant_square, Piece captured_piece);
        MoveUndoInfo& pop_undo_info();

    };

    std::ostream& operator<<(std::ostream& os, const Position& pos);
}



