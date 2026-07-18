// Crystall is a hobby UCI chess engine written in C++
// Developed by GoobusTheNoobus

#include "position.hpp"
#include "bitboards.hpp"
#include "attacks.hpp"
#include "zobrist.hpp"
#include "movelist.hpp"

#include <sstream>
#include <charconv>

namespace Crystall {
    constexpr char PieceCharacters[] = "PNBRQKpnbrqk";

    std::ostream& operator<<(std::ostream& os, const Position& pos) {
        os << pos.to_string();
        return os;
    }

    Position::Position() {
        Bitboards::init();
        Zobrist::init();
        parse_fen(StartingPositionFen);
    }

    void Position::parse_fen(const std::string& fen) {
        clear();

        std::istringstream iss(fen);

        std::string fen_board_part;
        if (!(iss >> fen_board_part)) return;

        int r = 7, f = 0;
        for (char c : fen_board_part) {
            if (c == '/') {
                --r;
                f = 0;
                continue;
            }

            if (std::isdigit(static_cast<unsigned char>(c))) {
                f += c - '0';
                continue;
            }

            Piece p = NoPiece;

            for (int i = 0; i < 13; ++i) {
                if (PieceCharacters[i] == c) {
                    p = Piece(i);
                    break;
                }
            }

            place_piece(make_square(r, f), p);
            ++f;
        }

        std::string fen_side_part;
        if (!(iss >> fen_side_part)) return;
        side_to_move = (fen_side_part == "w") ? White : Black;

        std::string fen_castling_part;
        if (!(iss >> fen_castling_part)) return;
        state.castling_rights = 0;

        if (fen_castling_part != "-") {
            for (char c : fen_castling_part) {
                switch (c) {
                    case 'K': state.castling_rights |= CastlingWK; break;
                    case 'Q': state.castling_rights |= CastlingWQ; break;
                    case 'k': state.castling_rights |= CastlingBK; break;
                    case 'q': state.castling_rights |= CastlingBQ; break;
                }
            }
        }

        std::string fen_ep_part;
        if (!(iss >> fen_ep_part)) return;

        if (fen_ep_part == "-")
            state.en_passant_square = NoSquare;
        else
            state.en_passant_square = make_square(fen_ep_part);

        std::string fen_rule50_part;
        if (!(iss >> fen_rule50_part)) return;
        std::from_chars(
            fen_rule50_part.data(),
            fen_rule50_part.data() + fen_rule50_part.size(),
            state.rule50_clock
        );

        if (side_to_move == Black)
            hash ^= Zobrist::SideKey;

        if (state.castling_rights != 0)
            hash ^= Zobrist::CastlingKeys[state.castling_rights];

        if (state.en_passant_square != NoSquare)
            hash ^= Zobrist::EnPassantKeys[file_of(state.en_passant_square)];

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

                if (piece != NoPiece) oss << PieceCharacters[piece];
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
        for (int i = 0; i < SquareNB; ++i) board[i] = NoPiece;
        for (int i = 0; i < PieceNB; ++i) piece_bitboards[i] = 0;
        for (int i = 0; i < ColorNB; ++i) color_bitboards[i] = 0;
        occupancy = 0;

        side_to_move = White;
        state.castling_rights = 0;
        state.en_passant_square = NoSquare;
        state.rule50_clock = 0;
        ply = 0;
        hash = 0;
        psqt_scores = { 0, 0 };
    }

    void Position::clear_square(Square square) {
        if (get_piece_on(square) == NoPiece) return;

        Piece piece_already_there = get_piece_on(square);
        Color color = color_of(piece_already_there);

        u64 mask = ~(1ULL << square);

        board[square] = NoPiece;

        piece_bitboards[piece_already_there] &= mask;
        color_bitboards[color] &= mask;
        occupancy &= mask;

        PieceType pt = type_of(piece_already_there);
        if (color == White) {
            psqt_scores.mg_score -= Evaluation::MGTables[pt][square ^ 56];
            psqt_scores.eg_score -= Evaluation::EGTables[pt][square ^ 56];
        } else {
            psqt_scores.mg_score += Evaluation::MGTables[pt][square];
            psqt_scores.eg_score += Evaluation::EGTables[pt][square];
        }

        hash ^= Zobrist::PieceSquareKeys[piece_already_there][square];
    }

    void Position::place_piece(Square square, Piece piece) {
        if (piece == NoPiece) {
            clear_square(square);
            return;
        }

        Color color = color_of(piece);

        u64 mask = 1ULL << square;

        board[square] = piece;
        piece_bitboards[piece] |= mask;
        color_bitboards[color] |= mask;
        occupancy |= mask;

        PieceType pt = type_of(piece);
        if (color == White) {
            psqt_scores.mg_score += Evaluation::MGTables[pt][square ^ 56];
            psqt_scores.eg_score += Evaluation::EGTables[pt][square ^ 56];
        } else {
            psqt_scores.mg_score -= Evaluation::MGTables[pt][square];
            psqt_scores.eg_score -= Evaluation::EGTables[pt][square];
        }

        hash ^= Zobrist::PieceSquareKeys[piece][square];
    }

    bool Position::is_attacked(Square square, Color by) const {
        return Attack::is_attacked(*this, square, by);
    }

    bool Position::is_in_check(Color color) const {
        return is_attacked(Square(ctz(get_bitboard(King, color))), opposite(color));
    }

    bool Position::is_in_check() const {
        return is_in_check(side_to_move);
    }

    void Position::push_move_stacks(u64 key, u16 move, int castling_rights, int rule50_clock, Square en_passant_square, Piece captured_piece) {
        move_undo_stack[ply++] = { key, castling_rights, rule50_clock, move, en_passant_square, captured_piece };
    }

    MoveUndoInfo& Position::pop_undo_info() {
        return move_undo_stack[--ply];
    }

    void Position::make_move(const u16 move) {
        Color us = side_to_move;
        bool is_white = us == White;

        Square from = Move::from(move);
        Square dest = Move::dest(move);
        Move::Type flag = Move::type(move);

        Piece moving_piece = NoPiece;
        PieceType moving_pt = Pawn;
        Piece captured_piece = NoPiece;

        if (!(move == Move::NullMove)) {
            moving_piece = get_piece_on(from);
            moving_pt = type_of(moving_piece);
            captured_piece = flag == Move::EnPassant
                ? make_piece(Pawn, opposite(us))
                : get_piece_on(dest);
        }

        u64 hash_before = hash;
        push_move_stacks(hash, move, state.castling_rights, state.rule50_clock,
                        state.en_passant_square, captured_piece);

        hash ^= Zobrist::SideKey;

        if (state.en_passant_square != NoSquare)
            hash ^= Zobrist::EnPassantKeys[file_of(state.en_passant_square)];

        if (state.castling_rights != 0)
            hash ^= Zobrist::CastlingKeys[state.castling_rights];

        side_to_move = opposite(side_to_move);
        state.en_passant_square = NoSquare;

        if (move == Move::NullMove) {
            return;
        }

        switch (flag) {
            case Move::Normal: {
                clear_square(dest);
                clear_square(from);
                place_piece(dest, moving_piece);
                break;
            }

            case Move::Castling: {
                bool king_side = dest == G1 || dest == G8;

                Square rook_from = is_white ? (king_side ? H1 : A1) : (king_side ? H8 : A8);
                Square rook_dest = is_white ? (king_side ? F1 : D1) : (king_side ? F8 : D8);

                clear_square(rook_from);
                clear_square(from);

                place_piece(dest, moving_piece);
                place_piece(rook_dest, make_piece(Rook, us));
                break;
            }

            case Move::EnPassant: {
                Square capture_square = is_white ? Square(dest - 8) : Square(dest + 8);

                clear_square(capture_square);
                clear_square(from);
                place_piece(dest, moving_piece);
                break;
            }

            case Move::DoublePawnPush: {
                state.en_passant_square = is_white ? Square(dest - 8) : Square(dest + 8);

                clear_square(from);
                place_piece(dest, moving_piece);
                break;
            }

            default: {
                constexpr static PieceType PromoPieces[] = {Queen, Rook, Bishop, Knight};

                clear_square(from);
                clear_square(dest);
                place_piece(dest, make_piece(PromoPieces[flag - Move::PromoQ], us));
                break;
            }
        }

        if (from == A1 || dest == A1) state.castling_rights &= ~CastlingWQ;
        if (from == A8 || dest == A8) state.castling_rights &= ~CastlingBQ;
        if (from == H1 || dest == H1) state.castling_rights &= ~CastlingWK;
        if (from == H8 || dest == H8) state.castling_rights &= ~CastlingBK;

        if (from == E1) state.castling_rights &= ~(CastlingWK | CastlingWQ);
        else if (from == E8) state.castling_rights &= ~(CastlingBK | CastlingBQ);

        if (state.en_passant_square != NoSquare)
            hash ^= Zobrist::EnPassantKeys[file_of(state.en_passant_square)];

        if (state.castling_rights != 0)
            hash ^= Zobrist::CastlingKeys[state.castling_rights];

        if (captured_piece != NoPiece || moving_pt == Pawn) state.rule50_clock = 0;
        else state.rule50_clock++;

    }

    void Position::make_move(const std::string& move_str) {
        MoveList list(*this);

        for (int i = 0; i < list.size(); ++i) {
            if (Move::to_string(list[i]) == move_str) {
                make_move(list[i]);
                return;
            }
        }
    }

    bool Position::attempt_move(const u16 move) {
        Color us = side_to_move;

        make_move(move);

        if (is_in_check(us)) {
            undo_move();
            return false;
        }

        return true;
    }

    void Position::undo_move() {
        side_to_move = opposite(side_to_move);

        Color us = side_to_move;
        bool is_white = us == White;

        MoveUndoInfo& info = pop_undo_info();

        u16 move = info.move;
        state.castling_rights = info.castling_rights;
        state.en_passant_square = info.en_passant_square;
        state.rule50_clock = info.rule50_clock;

        Piece captured_piece = info.captured_piece;

        Square from = Move::from(move);
        Square dest = Move::dest(move);
        Move::Type flag = Move::type(move);

        if (move == Move::NullMove) {
            hash = info.key;
            return;
        }

        Piece moving_piece = flag >= Move::PromoQ ? make_piece(Pawn, us) : get_piece_on(dest);

        switch (flag) {
            case Move::DoublePawnPush:
            case Move::Normal: {
                clear_square(dest);
                place_piece(from, moving_piece);
                if (captured_piece != NoPiece) place_piece(dest, captured_piece);
                break;
            }

            case Move::Castling: {
                bool king_side = dest == G1 || dest == G8;

                Square rook_from = is_white ? (king_side ? H1 : A1) : (king_side ? H8 : A8);
                Square rook_dest = is_white ? (king_side ? F1 : D1) : (king_side ? F8 : D8);

                clear_square(dest);
                clear_square(rook_dest);
                place_piece(from, moving_piece);
                place_piece(rook_from, make_piece(Rook, us));
                break;
            }

            case Move::EnPassant: {
                clear_square(dest);
                place_piece(from, moving_piece);
                place_piece(Square(is_white ? dest - 8 : dest + 8), captured_piece);
                break;
            }

            default: {
                clear_square(dest);
                place_piece(from, moving_piece);
                place_piece(dest, captured_piece);
                break;
            }
        }

        hash = info.key;
    }

    int Position::evaluate() const {
        int phase = Evaluation::calculate_phase(piece_bitboards);

        int score;
        score = psqt_scores.get_score(phase);

        return std::min(std::max((side_to_move == White ? score : -score) + Evaluation::TempoBonus, MinCentipawn), MaxCentipawn);
    } 

    bool Position::is_repetition() const {
        u64 key = hash;

        if (ply < 2) return false;

        int count = 0;

        for (int i = ply - 2; i >= std::max(0, ply - state.rule50_clock); i -= 2) {
            if (move_undo_stack[i].key == key) {
                ++count;
            }
            if (count >= 2) {
                return true;
            }
        }

        return false;
    }

    bool Position::has_non_pawn_material() const {
        return color_bitboards[side_to_move] & ~(piece_bitboards[make_piece(Pawn, side_to_move)] | piece_bitboards[make_piece(King, side_to_move)]);
    }

}
