#pragma once

#include "types.hpp"
#include "move.hpp"
#include <string>
#include <iostream>

namespace Crystall {

    // position representation
    struct GameState {
        Square en_passant_square = NO_SQUARE;
        int castling_rights = 0;
        int rule50_clock = 0;
    };

    // a struct that represents the information needed to undo 
    // a move. should be stored in stack
    struct MoveUndoInfo {
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
        Piece board[SQUARE_NB];
        u64 piece_bitboards[PIECE_NB];
        u64 color_bitboards[COLOR_NB];
        u64 occupancy = 0;

        // game states
        Color side_to_move;
        GameState state;

        // stack(s)
        MoveUndoInfo move_undo_stack[1024];
        int ply = 0;

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
        void make_move(Move);
        bool attempt_move(Move);
        void undo_move();
        
        bool equals(const Position& other) const {
            if (side_to_move != other.side_to_move) return false;
            if (state.en_passant_square != other.state.en_passant_square) return false;
            if (state.castling_rights != other.state.castling_rights) return false;
            if (state.rule50_clock != other.state.rule50_clock) return false;
            if (occupancy != other.occupancy) return false;
            if (ply != other.ply) return false;

            for (int i = 0; i < SQUARE_NB; ++i) {
                if (board[i] != other.board[i]) return false;
            }

            for (int i = 0; i < PIECE_NB; ++i) {
                if (piece_bitboards[i] != other.piece_bitboards[i]) return false;
            }

            for (int i = 0; i < COLOR_NB; ++i) {
                if (color_bitboards[i] != other.color_bitboards[i]) return false;
            }

            return true;
        }

        void print_differences(const Position& other) const {
            bool same = true;

            if (side_to_move != other.side_to_move) {
                std::cout << "side_to_move differs: "
                        << int(side_to_move) << " vs " << int(other.side_to_move) << '\n';
                same = false;
            }

            if (state.en_passant_square != other.state.en_passant_square) {
                std::cout << "en_passant_square differs: "
                        << int(state.en_passant_square) << " vs " << int(other.state.en_passant_square) << '\n';
                same = false;
            }

            if (state.castling_rights != other.state.castling_rights) {
                std::cout << "castling_rights differs: "
                        << state.castling_rights << " vs " << other.state.castling_rights << '\n';
                same = false;
            }

            if (state.rule50_clock != other.state.rule50_clock) {
                std::cout << "rule50_clock differs: "
                        << state.rule50_clock << " vs " << other.state.rule50_clock << '\n';
                same = false;
            }

            if (occupancy != other.occupancy) {
                std::cout << "occupancy differs: "
                        << occupancy << " vs " << other.occupancy << '\n';
                same = false;
            }

            if (ply != other.ply) {
                std::cout << "ply differs: "
                        << ply << " vs " << other.ply << '\n';
                same = false;
            }

            for (int i = 0; i < SQUARE_NB; ++i) {
                if (board[i] != other.board[i]) {
                    std::cout << "board[" << i << "] differs: "
                            << int(board[i]) << " vs " << int(other.board[i]) << '\n';
                    same = false;
                }
            }

            for (int i = 0; i < PIECE_NB; ++i) {
                if (piece_bitboards[i] != other.piece_bitboards[i]) {
                    std::cout << "piece_bitboards[" << i << "] differs: "
                            << piece_bitboards[i] << " vs " << other.piece_bitboards[i] << '\n';
                    same = false;
                }
            }

            for (int i = 0; i < COLOR_NB; ++i) {
                if (color_bitboards[i] != other.color_bitboards[i]) {
                    std::cout << "color_bitboards[" << i << "] differs: "
                            << color_bitboards[i] << " vs " << other.color_bitboards[i] << '\n';
                    same = false;
                }
            }

            if (same) {
                std::cout << "Positions are identical.\n";
            }
        }

        // helper functions
        private:

        void clear();
        void clear_square(Square square);
        void place_piece(Square square, Piece piece);

        void push_undo_info(Move, int castling_rights, int rule50_clock, Square en_passant_square, Piece captured_piece);
        MoveUndoInfo& pop_undo_info();

    };

    std::ostream& operator<<(std::ostream& os, const Position& pos);
}



