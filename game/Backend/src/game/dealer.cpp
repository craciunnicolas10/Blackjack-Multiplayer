#include <Backend/game/dealer.h>

#include <Backend/game/hand_score.h>

namespace Backend {
    namespace {
        constexpr int kHoleCardIndex = 1;  // a doua carte a dealerului
    }  // namespace

    Dealer::Dealer() : Player("Dealer", 0) {}

    std::vector<Card> Dealer::GetHand() const {
        std::vector<Card> visible = GetFullHand();
        if (m_holeCardHidden) {
            visible.erase(visible.begin() + kHoleCardIndex);
        }
        return visible;
    }

    int Dealer::GetScore() const { return ScoreHand(GetHand()); }

    void Dealer::AddHoleCard(Card card) {
        AddCard(card);
        m_holeCardHidden = true;
    }

    bool Dealer::HasHiddenCard() const { return m_holeCardHidden; }

    Card Dealer::RevealHoleCard() {
        m_holeCardHidden = false;
        return GetFullHand()[kHoleCardIndex];
    }
}  // namespace Backend
