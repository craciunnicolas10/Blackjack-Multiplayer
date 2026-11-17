#include <ui/card_view.h>

#include <algorithm>
#include <cmath>
#include <cstdint>

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <ui/card_textures.h>
#include <ui/theme.h>

namespace Frontend {
    namespace {
        // Pornește repede și încetinește la final, ca o carte aruncată pe masă.
        float easeOutCubic(float t) {
            const float inverse = 1.f - t;
            return 1.f - inverse * inverse * inverse;
        }
    }  // namespace

    CardView::CardView(const CardTextures& textures, int rank, int suit, bool faceUp)
        : m_textures(&textures), m_rank(rank), m_suit(suit), m_faceUp(faceUp) {}

    CardView::CardView(const CardTextures& textures) : m_textures(&textures) {}

    void CardView::setCard(int rank, int suit) {
        m_rank = rank;
        m_suit = suit;
    }

    bool CardView::isKnown() const { return m_suit >= 0; }

    int CardView::getRank() const { return m_rank; }

    int CardView::getSuit() const { return m_suit; }

    void CardView::setFaceUp(bool faceUp) {
        m_faceUp = faceUp;
        m_flipping = false;
    }

    bool CardView::isFaceUp() const { return m_faceUp; }

    void CardView::flip(float duration) {
        if (m_flipping && !m_flipSwapped) {
            // O întoarcere începută și neterminată: o terminăm întâi.
            m_faceUp = !m_faceUp;
        }
        if (duration <= 0.f) {
            setFaceUp(!m_faceUp);
            return;
        }
        m_flipping = true;
        m_flipSwapped = false;
        m_flipElapsed = 0.f;
        m_flipDuration = duration;
    }

    void CardView::setPosition(sf::Vector2f position) {
        m_position = position;
        m_moving = false;
    }

    sf::Vector2f CardView::getPosition() const { return m_position; }

    void CardView::moveTo(sf::Vector2f target, float duration, float delay) {
        if (duration <= 0.f && delay <= 0.f) {
            setPosition(target);
            return;
        }
        m_moveFrom = m_position;
        m_moveTarget = target;
        m_moveElapsed = 0.f;
        m_moveDuration = std::max(duration, 0.f);
        m_moveDelay = std::max(delay, 0.f);
        m_moving = true;
    }

    void CardView::retarget(sf::Vector2f target) {
        if (!m_moving) {
            moveTo(target);
            return;
        }
        if (m_moveDelay > 0.f) {
            // N-a plecat încă: schimbăm doar destinația.
            m_moveTarget = target;
            return;
        }
        // E pe drum: pleacă din punctul curent spre noua țintă, cât timp îi mai rămăsese.
        const float remaining = std::max(m_moveDuration - m_moveElapsed, 0.f);
        moveTo(target, remaining);
    }

    bool CardView::isMoving() const { return m_moving; }

    bool CardView::isWaitingInDeck() const { return m_moving && m_moveDelay > 0.f; }

    bool CardView::isAnimating() const { return m_moving || m_flipping; }

    void CardView::update(float dt) {
        if (m_moving) {
            float moveDt = dt;
            if (m_moveDelay > 0.f) {
                m_moveDelay -= moveDt;
                // Ce a rămas din frame după ce s-a terminat întârzierea se folosește la mișcare.
                moveDt = m_moveDelay < 0.f ? -m_moveDelay : 0.f;
                m_moveDelay = std::max(m_moveDelay, 0.f);
            }
            if (m_moveDelay <= 0.f) {
                m_moveElapsed += moveDt;
                const float t =
                    m_moveDuration > 0.f ? std::min(m_moveElapsed / m_moveDuration, 1.f) : 1.f;
                m_position = m_moveFrom + (m_moveTarget - m_moveFrom) * easeOutCubic(t);
                if (t >= 1.f) {
                    m_position = m_moveTarget;
                    m_moving = false;
                }
            }
        }

        if (m_flipping) {
            m_flipElapsed += dt;
            // La jumătate, cartea e văzută din profil: atunci schimbăm fața cu spatele.
            if (!m_flipSwapped && m_flipElapsed >= m_flipDuration / 2.f) {
                m_faceUp = !m_faceUp;
                m_flipSwapped = true;
            }
            if (m_flipElapsed >= m_flipDuration) {
                m_flipping = false;
            }
        }
    }

    void CardView::draw(sf::RenderTarget& target, sf::RenderStates states) const {
        // Cât timp așteaptă în pachet să fie împărțită, cartea stă cu spatele în sus, ca să nu
        // se vadă dinainte ce urmează.
        const bool waitingInDeck = isWaitingInDeck();
        const bool showFace = m_faceUp && isKnown() && !waitingInDeck;
        const sf::Texture& texture =
            showFace ? m_textures->get(m_rank, m_suit) : m_textures->back();

        const sf::Vector2f textureSize(texture.getSize());
        if (textureSize.x <= 0.f || textureSize.y <= 0.f) {
            return;
        }

        // Întoarcerea: lățimea scade la 0 până la jumătate, apoi crește la loc.
        float widthFactor = 1.f;
        if (m_flipping) {
            const float t = std::min(m_flipElapsed / m_flipDuration, 1.f);
            widthFactor = std::abs(1.f - 2.f * t);
        }

        // Umbra ușoară de sub carte. Cât timp cartea e în aer (alunecă), umbra e mai departe și
        // mai slabă, ca și cum ar fi ridicată de pe masă.
        const bool inAir = m_moving && !waitingInDeck;
        const sf::Vector2f shadowOffset = inAir ? sf::Vector2f{7.f, 9.f} : sf::Vector2f{3.f, 4.f};
        const std::uint8_t shadowAlpha = inAir ? 55 : 80;
        // Puțin mai mică decât cartea, ca să nu iasă pe la colțurile rotunjite.
        const sf::Vector2f shadowSize{(kSize.x - 4.f) * widthFactor, kSize.y - 4.f};
        sf::RectangleShape shadow(shadowSize);
        shadow.setOrigin(shadowSize / 2.f);
        shadow.setPosition(m_position + kSize / 2.f + shadowOffset);
        sf::Color shadowColor = Theme::kShadow;
        shadowColor.a = shadowAlpha;
        shadow.setFillColor(shadowColor);
        target.draw(shadow, states);

        sf::Sprite sprite(texture);
        sprite.setOrigin(textureSize / 2.f);
        sprite.setScale({kSize.x / textureSize.x * widthFactor, kSize.y / textureSize.y});
        sprite.setPosition(m_position + kSize / 2.f);
        target.draw(sprite, states);
    }
}  // namespace Frontend
