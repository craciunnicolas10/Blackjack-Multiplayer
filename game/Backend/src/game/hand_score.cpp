#include <Backend/game/hand_score.h>

namespace Backend {
    namespace {
        constexpr int kAceHighValue = 11;
        constexpr int kAceHighBonus = kAceHighValue - 1;  // as numarat 11 in loc de 1
        constexpr int kFaceCardValue = 10;
    }  // namespace

    int CardValue(ERank rank) {
        if (rank == ERank::Ace) {
            return kAceHighValue;
        }
        if (rank >= ERank::Jack) {
            return kFaceCardValue;
        }
        return static_cast<int>(rank);
    }

    int ScoreHand(const std::vector<Card>& hand) {
        int score = 0;
        int highAces = 0;
        for (const Card& card : hand) {
            score += CardValue(card.rank);
            if (card.rank == ERank::Ace) {
                ++highAces;
            }
        }
        while (score > kBlackjackScore && highAces > 0) {
            score -= kAceHighBonus;
            --highAces;
        }
        return score;
    }

    bool IsBlackjack(const std::vector<Card>& hand) {
        return hand.size() == 2 && ScoreHand(hand) == kBlackjackScore;
    }
}  // namespace Backend
