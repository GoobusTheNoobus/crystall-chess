#include "position.hpp"

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
}