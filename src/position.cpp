#include "position.hpp"

#include <sstream>
#include <iostream>

namespace Crystall 
{

    constexpr char PIECE_CHARACTERS[] = "PNBRQKpnbrqk.";
    constexpr char STARTING_POSITION[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    // Default constructor sets up starting position
    Position::Position() 
    {
        parse_fen(STARTING_POSITION);
    }

    // Parses a FEN position. 
    // An invalid FEN leads to undefined behavior
    void Position::parse_fen(const std::string& fen) 
    {
        clear();

        std::istringstream iss(fen);

        std::string fen_board_part;
        if (!(iss >> fen_board_part)) return;

        int r = 7, f = 0;
        for (char c : fen_board_part) 
        {
            if (c == '/') 
            {
                r -= 1;
                f = 0;
                continue;
            }

            if (std::isdigit(c)) 
            {
                f += c - '0';
                continue;
            }

            Piece p = Piece::None;
            int i = 0;
            for (int i = 0; i < 13; ++i) 
            {
                if (PIECE_CHARACTERS[i] == c) 
                {
                    p = Piece(i);
                }
            }

            place_piece(make_square(r, f), p);
            ++f;
        }

        std::string fen_side_part;
        if (!(iss >> fen_side_part)) return;

        side_to_move = fen_side_part == "w" ? Color::White : Color::Black;

        std::string fen_castling_part;
        if (!(iss >> fen_castling_part)) return;

        for (char c : fen_castling_part) 
        {
            switch (c) 
            {
                case 'K': state.castling_rights |= Castling::WHITE_KINGSIDE; break;
                case 'Q': state.castling_rights |= Castling::WHITE_QUEENSIDE; break;
                case 'k': state.castling_rights |= Castling::BLACK_KINGSIDE; break;
                case 'q': state.castling_rights |= Castling::BLACK_QUEENSIDE; break;
                default:
                    return;
            }
        }

        std::string fen_ep_part;
        if (!(iss >> fen_ep_part)) return;

        state.en_passant_square = make_square(fen_ep_part);

        std::string fen_rule50_part;
        if (!(iss >> fen_rule50_part)) return;

        state.rule50_clock = std::atoi(fen_rule50_part.data());
    }

    void Position::clear() 
    {
        for (int i = 0; i < SQUARE_NB; ++i) board[i] = Piece::None;      
        for (int i = 0; i < PIECE_NB; ++i) piece_bitboards[i] = 0;
        for (int i = 0; i < COLOR_NB; ++i) color_bitboards[i] = 0;
        occupancy = 0;
        
        side_to_move = Color::White;
        state.castling_rights = 0;
        state.en_passant_square = Square::None;
        state.rule50_clock = 0;
    }

    void Position::clear_square(Square square) 
    {
        if (get_piece_on(square) == Piece::None) return;

        Piece piece_already_there = get_piece_on(square);
        Color color = color_of(piece_already_there);

        Bitboard mask = ~(1ULL << int(square));

        board[int(square)] = Piece::None;

        piece_bitboards[int(piece_already_there)] &= mask;
        color_bitboards[int(color)] &= mask;
        occupancy &= mask;
    }

    // This assumes that the square is empty
    // If you are not sure that the square is empty, 
    // clear it first in case
    void Position::place_piece(Square square, Piece piece) 
    {
        if (piece == Piece::None) 
        {
            clear_square(square);
            return;
        }

        Color color = color_of(piece);

        Bitboard mask = 1ULL << int(square);

        board[int(square)] = piece;

        piece_bitboards[int(piece)] |= mask;
        color_bitboards[int(color)] |= mask;
        occupancy |= mask;
    }

    std::string Position::to_string() const 
    {
        std::ostringstream oss;

        for (int r = 7; r >= 0; --r) 
        {
            oss << "  +---+---+---+---+---+---+---+---+\n";
            oss << (r + 1) << ' ';
            for (int f = 0; f < 8; ++f) 
            {
                oss << "| ";

                Piece piece = get_piece_on(make_square(r, f));

                if (piece != Piece::None) oss << PIECE_CHARACTERS[int(piece)];
                else oss << ' ';

                oss << ' ';
            }
            oss << "|\n";
        }

        oss << "  +---+---+---+---+---+---+---+---+\n";
        oss << "    a   b   c   d   e   f   g   h  \n\n";

        return oss.str();
    }

    std::ostream& operator<<(std::ostream& os, const Position& pos) {
        os << pos.to_string();
        return os;
    }
}