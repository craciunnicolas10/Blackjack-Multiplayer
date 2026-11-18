#include <Backend/game/deck.h>

#include <algorithm>

namespace Backend {
    Deck::Deck() : m_rng(std::random_device{}()) { Shuffle(); }

    void Deck::Shuffle() {
        m_cards.clear();
        for (int suit = static_cast<int>(ESuit::Clubs); suit <= static_cast<int>(ESuit::Spades);
             ++suit) {
            for (int rank = static_cast<int>(ERank::Two); rank <= static_cast<int>(ERank::Ace);
                 ++rank) {
                m_cards.push_back({static_cast<ERank>(rank), static_cast<ESuit>(suit)});
            }
        }
        std::shuffle(m_cards.begin(), m_cards.end(), m_rng);
    }

    Card Deck::Draw() {
        if (m_cards.empty()) {
            Shuffle();
        }
        Card card = m_cards.back();
        m_cards.pop_back();
        return card;
    }
}  // namespace Backend
