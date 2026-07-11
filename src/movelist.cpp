#include "movelist.hpp"
#include "search.hpp"

namespace Crystall {

    const Move Move::NullMove {};

    namespace {

        // Move Valuable Victim to Least Valuable Attacker
        // Used for move ordering
        constexpr int MVVLVATable[PieceTypeNB][PieceTypeNB] = {
            { 809000, 831000, 832000, 849000, 889000, 999000 },
            { 806800, 828800, 829800, 846800, 886800, 996800 },
            { 806700, 828700, 829700, 846700, 886700, 996700 },
            { 805000, 827000, 828000, 845000, 885000, 995000 },
            { 801000, 823000, 824000, 841000, 881000, 991000 },
            { 799000, 821000, 822000, 839000, 879000, 989000 }
        };

        constexpr int PromotionScoreTable[4] = { 790000, 750000, 732000, 731000 };
        
        constexpr int MaxMoveScore = 1000000;
        int score_move(const Position& pos, const Move& move, const Move& special_move) {
            if (move == special_move) return MaxMoveScore;
            
            Square from = move.from();
            Square dest = move.dest();
            Move::Type flag = move.flag();

            if (pos.get_piece_on(dest) != NoPiece) {
                int mvvlva_score = MVVLVATable[type_of(pos.get_piece_on(from))][type_of(pos.get_piece_on(dest))];
                return mvvlva_score;
            }

            if (flag >= Move::PromoQ) {
                int promo_score = PromotionScoreTable[flag - Move::PromoQ];
                return promo_score;
            }

            int history_score = Search::history_table[pos.get_side_to_move()][from][dest];
            return history_score;
        }
    }

    void MoveList::calculate_scores(const Move& special_move) {
        for (int i = 0; i < size_; ++i) {
            scores[i] = score_move(pos, moves[i], special_move);
        }
    }

    void MoveList::calculate_scores() {
        for (int i = 0; i < size_; ++i) {
            scores[i] = score_move(pos, moves[i], Move::NullMove);
        }
    }
}