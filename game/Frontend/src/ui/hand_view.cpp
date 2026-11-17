#include <ui/hand_view.h>

#include <algorithm>
#include <utility>

namespace Frontend {
    HandView::HandView(const CardTextures& textures) : m_textures(&textures) {}

    void HandView::setPosition(sf::Vector2f centerTop) {
        m_position = centerTop;
        relayout(false);
    }

    sf::Vector2f HandView::getPosition() const { return m_position; }

    void HandView::addCard(int rank, int suit, bool faceUp) {
        m_cards.emplace_back(*m_textures, rank, suit, faceUp);
        relayout(false);
    }

    void HandView::addHiddenCard() {
        m_cards.emplace_back(*m_textures);
        relayout(false);
    }

    void HandView::dealCard(int rank, int suit, bool faceUp, sf::Vector2f from, float delay) {
        placeCard(CardView(*m_textures, rank, suit, faceUp), from, delay);
    }

    void HandView::dealHiddenCard(sf::Vector2f from, float delay) {
        placeCard(CardView(*m_textures), from, delay);
    }

    void HandView::revealCard(std::size_t index, int rank, int suit) {
        CardView& card = m_cards.at(index);
        card.setCard(rank, suit);
        if (!card.isFaceUp()) {
            card.flip();
        }
    }

    void HandView::clear() { m_cards.clear(); }

    std::size_t HandView::getCardCount() const { return m_cards.size(); }

    const CardView& HandView::getCard(std::size_t index) const { return m_cards.at(index); }

    CardView& HandView::getCard(std::size_t index) { return m_cards.at(index); }

    sf::FloatRect HandView::getBounds() const {
        if (m_cards.empty()) {
            return {m_position, {0.f, 0.f}};
        }
        const sf::Vector2f topLeft = slotPosition(0);
        const float width =
            CardView::kSize.x + kCardOffset * static_cast<float>(m_cards.size() - 1);
        return {topLeft, {width, CardView::kSize.y}};
    }

    bool HandView::isAnimating() const {
        return std::any_of(m_cards.begin(), m_cards.end(),
                           [](const CardView& card) { return card.isAnimating(); });
    }

    void HandView::update(float dt) {
        for (CardView& card : m_cards) {
            card.update(dt);
        }
    }

    void HandView::draw(sf::RenderTarget& target) const {
        // În ordine: ultima carte e deasupra celorlalte.
        for (const CardView& card : m_cards) {
            card.draw(target);
        }
    }

    sf::Vector2f HandView::slotPosition(std::size_t index) const {
        const float width =
            CardView::kSize.x + kCardOffset * static_cast<float>(m_cards.size() - 1);
        const float left = m_position.x - width / 2.f;
        return {left + kCardOffset * static_cast<float>(index), m_position.y};
    }

    void HandView::placeCard(CardView card, sf::Vector2f from, float delay) {
        card.setPosition(from);
        m_cards.push_back(std::move(card));
        relayout(true);
        m_cards.back().moveTo(slotPosition(m_cards.size() - 1), CardView::kMoveDuration, delay);
    }

    void HandView::relayout(bool animate) {
        for (std::size_t i = 0; i < m_cards.size(); ++i) {
            CardView& card = m_cards[i];
            if (card.isMoving()) {
                // E încă pe drum (sau așteaptă să plece din pachet): doar îi schimbăm ținta.
                card.retarget(slotPosition(i));
            } else if (animate) {
                card.moveTo(slotPosition(i));
            } else {
                card.setPosition(slotPosition(i));
            }
        }
    }
}  // namespace Frontend
